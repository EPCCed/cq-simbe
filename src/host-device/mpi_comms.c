#include "mpi_comms.h"

#include "datatypes.h"
#include "src/host-device/comms.h"
#include "src/host/opcodes.h"

#include <assert.h>
#include <mpi.h>
#include <mpi_proto.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static MPI_Comm CQ_MPI_COMM = MPI_COMM_WORLD;

struct cq_mpi_env {
  int rank;
};

static cq_exec* executor_handles[__CQ_DEVICE_QUEUE_SIZE__];
static int executor_id = 0;
static struct cq_mpi_env mpi_env = {.rank = -1};

void init_host_device_mpi(const unsigned int VERBOSITY) {
  if (VERBOSITY > 0) {
    printf("Initialising MPI.\n");
  }
  MPI_Init(NULL, NULL);
  MPI_Comm_rank(CQ_MPI_COMM, &mpi_env.rank);
  if (VERBOSITY > 0) {
    printf("Initialised MPI.\n");
  }

  if (mpi_env.rank != CQ_MPI_HOST_RANK) {
    dev_ctrl.run_device = true;
    printf("%s started listening...\n", get_comm_source());
    device_listen();
    printf("%s closing connection.\n", get_comm_source());
    finalise_host_device_mpi(VERBOSITY);
  }
}

void finalise_host_device_mpi(const unsigned int VERBOSITY) {
  if (VERBOSITY > 0) {
    printf("%s Finalising MPI.\n", get_comm_source());
  }
  MPI_Finalize();
  if (VERBOSITY > 0) {
    printf("Finalised MPI.\n");
  }
}

// void mpi_host_send_ctrl_op(const enum ctrl_code OP,
//                            struct ctrl_params* params) {
void mpi_host_send_ctrl_op(const enum ctrl_code OP, void* params) {
  const int device_rank = CQ_MPI_DEVICE_RANK;
  printf("%s [send_ctrl_op]: sending %s...\n", get_comm_source(),
         op_to_str(OP));
  MPI_Ssend(&OP, 1, MPI_INT, device_rank, CQ_MPI_COMMS_TAG, CQ_MPI_COMM);
  // send params based on OP
  host_comm_params(OP, params);
  printf("%s [send_ctrl_op]: sent %s\n", get_comm_source(), op_to_str(OP));
}

void mpi_host_wait_all_ops(void) {
  printf("%s [wait_all_ops]: waiting...\n", get_comm_source());
  mpi_host_send_ctrl_op(CQ_CTRL_WAIT, NULL);
  size_t num_ops;
  const int device_rank = CQ_MPI_DEVICE_RANK;
  MPI_Status status;
  MPI_Recv(&num_ops, 1, MPI_UINT64_T, device_rank, CQ_MPI_COMMS_TAG,
           CQ_MPI_COMM, &status);
  printf("%s [wait_all_ops]: all ops completed (num_ops: %zu)\n",
         get_comm_source(), num_ops);
}

// ----------------------------------------------------------------------------
// Device Comm Ops
// ----------------------------------------------------------------------------

void device_listen(void) {
  // run_device set to FALSE when OP == CQ_CTRL_FINALISE
  while (dev_ctrl.run_device) {
    enum ctrl_code OP;
    const int host_rank = CQ_MPI_HOST_RANK;
    MPI_Status status;
    printf("%s [device_listen]: receiving OP...\n", get_comm_source());
    MPI_Recv(&OP, 1, MPI_INT, host_rank, CQ_MPI_COMMS_TAG, CQ_MPI_COMM,
             &status);
    printf("%s [device_listen]: received %s\n", get_comm_source(),
           op_to_str(OP));

    device_dispatch_ctrl_op(OP);
  }
}

// NOTE: aka host_send_ctrl_op from comm.c
// really what I want to use is host_send_ctrl_op implemenetation but it needs
// renaming and wrapping into function called host_send_ctrl_op and depending on
// either MPI or pthread implementation use this in different ctx.
void insert_op(const enum ctrl_code OP, void* ctrl_params) {
  pthread_mutex_lock(&dev_ctrl.device_lock);

  while (dev_ctrl.num_ops >= __CQ_DEVICE_QUEUE_SIZE__) {
    // the control queue is full!
    // we'll wait for it to not be full
    pthread_cond_wait(&dev_ctrl.cond_queue_full, &dev_ctrl.device_lock);
  }

  dev_ctrl.op_buffer[dev_ctrl.next_op_in] = OP;
  dev_ctrl.op_params_buffer[dev_ctrl.next_op_in] = ctrl_params;
  ++dev_ctrl.num_ops;

  // It's a ring buffer!
  // advance next_op_in then mod out buffer size
  ++dev_ctrl.next_op_in;
  dev_ctrl.next_op_in %= __CQ_DEVICE_QUEUE_SIZE__;

  pthread_cond_signal(&dev_ctrl.cond_queue_empty);
  pthread_mutex_unlock(&dev_ctrl.device_lock);
}

// optionally get params and run op
// by running op I mean modyfing the internal dev_ctrl fields
// and then the worker thread handles the rest.
void device_dispatch_ctrl_op(const enum ctrl_code OP) {
  // TODO: needs to add extra CTRL ops for EXEC sync in opcodes.h
  // then modyfing host_sync_exec and host_wait_exec
  switch (OP) {
    case CQ_CTRL_INIT: {
      const unsigned int VERBOSITY = 1;
      initialise_device(VERBOSITY);
      insert_op(OP, &VERBOSITY);
      break;
    }
    case CQ_CTRL_FINALISE: {
      // don't need to insert op into worker.
      // we just wait until worker is done and cleanup.
      device_wait_all_ops();
      finalise_device(1);
      break;
    }
    case CQ_CTRL_ALLOC: {
      // Alloc is blocking: we get params, run allocation and
      // send back the updated params.
      // Also, because it's blocking I don't need to worry about
      // params lifetime (from the worker perspecitve)
      const int host_rank = CQ_MPI_HOST_RANK;
      device_alloc_params params = {0};
      recv_alloc_params(&params, host_rank);
      insert_op(OP, &params);
      device_wait_all_ops();
      send_alloc_params(&params, host_rank);
      break;
    }
    case CQ_CTRL_DEALLOC: {
      // Same as Alloc
      const int host_rank = CQ_MPI_HOST_RANK;
      device_alloc_params params = {0};
      recv_alloc_params(&params, host_rank);
      insert_op(OP, &params);
      device_wait_all_ops();
      send_alloc_params(&params, host_rank);
      break;
    }
    case CQ_CTRL_RUN_QKERNEL: {
      const int host_rank = CQ_MPI_HOST_RANK;
      recv_exec_params(executor_handles[executor_id], host_rank);
      insert_op(OP, executor_handles[executor_id]);
      break;
    }
    case CQ_CTRL_RUN_PQKERNEL: {
      // recv params
      // insert_op
      break;
    }
    case CQ_CTRL_WAIT_EXEC: {
      device_wait_all_ops();
      const int host_rank = CQ_MPI_HOST_RANK;
      send_exec_params(executor_handles[executor_id], host_rank);
      free(executor_handles[executor_id]);
      executor_handles[executor_id] = NULL;
      break;
    }
    case CQ_CTRL_WAIT: {
      // wait for worker and send info to the host
      // (which is blocked and waiting).
      size_t num_ops = device_wait_all_ops();
      const int host_rank = CQ_MPI_HOST_RANK;
      printf("%s [dispatch]: sending num ops: %zu...\n", get_comm_source(),
             num_ops);
      MPI_Ssend(&num_ops, 1, MPI_UINT64_T, host_rank, CQ_MPI_COMMS_TAG,
                CQ_MPI_COMM);
      printf("%s [dispatch]: sent num ops.\n", get_comm_source());
      break;
    }
    case CQ_CTRL_ABORT: {
      break;
    }
    case CQ_CTRL_IDLE: {
      break;
    }
    case CQ_CTRL_TEST: {
      break;
    }
    default: {
      break;
    }
  }
}

// NOTE: aka host_wait_all_ops from comm.c
// really what I want to use is host_wait_all_ops implementation but it needs
// renaming and wrapping into function called host_send_ctrl_op and depending on
// either MPI or pthread implementation use this in different ctx.
size_t device_wait_all_ops(void) {
  pthread_mutex_lock(&dev_ctrl.device_lock);
  while (dev_ctrl.num_ops > 0 || dev_ctrl.device_busy) {
    pthread_cond_wait(&dev_ctrl.cond_device_busy, &dev_ctrl.device_lock);
  }

  pthread_mutex_unlock(&dev_ctrl.device_lock);
  return dev_ctrl.num_ops;
}

// ----------------------------------------------------------------------------
// Device Control Paramaters Comms
// ----------------------------------------------------------------------------

void host_comm_params(const enum ctrl_code OP, void* params) {
  switch (OP) {
    case CQ_CTRL_INIT: {
      break;
    }
    case CQ_CTRL_FINALISE: {
      break;
    }
    case CQ_CTRL_ALLOC: {
      const int device_rank = CQ_MPI_DEVICE_RANK;
      send_alloc_params(params, device_rank);
      recv_alloc_params(params, device_rank);
      break;
    }
    case CQ_CTRL_DEALLOC: {
      const int device_rank = CQ_MPI_DEVICE_RANK;
      send_alloc_params(params, device_rank);
      recv_alloc_params(params, device_rank);
      break;
    }
    case CQ_CTRL_RUN_QKERNEL: {
      const int device_rank = CQ_MPI_DEVICE_RANK;
      send_exec_params(params, device_rank);
      break;
    }
    case CQ_CTRL_WAIT_EXEC: {
      const int device_rank = CQ_MPI_DEVICE_RANK;
      recv_exec_params(params, device_rank);
      break;
    }
    default: {
      break;
    }
  }
}

void recv_alloc_params(device_alloc_params* params, int src) {
  printf("%s [recv_alloc_params]: receiving...\n", get_comm_source());

  const size_t params_size = sizeof(device_alloc_params);
  MPI_Status status;

  MPI_Recv((char*)params, params_size, MPI_BYTE, src, CQ_MPI_COMMS_TAG,
           CQ_MPI_COMM, &status);

  printf("%s [recv_alloc_params]: received.\n", get_comm_source());
  print_alloc_params(params);
}

void send_alloc_params(const device_alloc_params* params, int dest) {
  printf("%s [send_alloc_params]: sending...\n", get_comm_source());
  const size_t params_size = sizeof(device_alloc_params);
  print_alloc_params(params);
  MPI_Ssend((char*)params, params_size, MPI_BYTE, dest, CQ_MPI_COMMS_TAG,
            CQ_MPI_COMM);
  printf("%s [send_alloc_params]: sent.\n", get_comm_source());
}

void recv_exec_params(cq_exec* ehp, int src) {
  printf("%s [recv_exec_params]: receiving...\n", get_comm_source());
  int msg_size;
  MPI_Status status;
  MPI_Recv(&msg_size, 1, MPI_INT, src, CQ_MPI_COMMS_TAG, CQ_MPI_COMM, &status);
  void* recv_buffer = malloc(msg_size);

  // reserve space for all the date + currently unused members
  if (ehp == NULL) {
    printf("%s [recv_exec_params]: ehp is NULL. Allocating on device.\n",
           get_comm_source());

    // ehp = (cq_exec*)malloc(msg_size + sizeof(pthread_mutex_t) +
    //                        sizeof(pthread_cond_t) + sizeof(void*));
    ehp = (cq_exec*)malloc(sizeof(cq_exec));
  } else {
    printf("%s [recv_exec_params]: ehp is already allocated. So I'm on host.\n",
           get_comm_source());
  }

  if (recv_buffer == NULL || ehp == NULL) {
    printf("%s [recv_exec_params]: malloc failed. Exiting\n",
           get_comm_source());
    exit(-10);
  }

  MPI_Recv(recv_buffer, msg_size, MPI_PACKED, src, CQ_MPI_COMMS_TAG,
           CQ_MPI_COMM, &status);

  const size_t bool_size = sizeof(bool);
  const size_t cq_status_size = sizeof(cq_status);
  // unpack
  int position = 0;
  MPI_Unpack(recv_buffer, msg_size, &position, &ehp->exec_init, bool_size,
             MPI_BYTE, CQ_MPI_COMM);
  MPI_Unpack(recv_buffer, msg_size, &position, &ehp->complete, bool_size,
             MPI_BYTE, CQ_MPI_COMM);
  MPI_Unpack(recv_buffer, msg_size, &position, &ehp->halt, bool_size, MPI_BYTE,
             CQ_MPI_COMM);
  MPI_Unpack(recv_buffer, msg_size, &position, &ehp->status, cq_status_size,
             MPI_BYTE, CQ_MPI_COMM);
  MPI_Unpack(recv_buffer, msg_size, &position, &ehp->nqubits, 1, MPI_UINT64_T,
             CQ_MPI_COMM);
  MPI_Unpack(recv_buffer, msg_size, &position, &ehp->completed_shots, 1,
             MPI_UINT64_T, CQ_MPI_COMM);
  MPI_Unpack(recv_buffer, msg_size, &position, &ehp->expected_shots, 1,
             MPI_UINT64_T, CQ_MPI_COMM);
  MPI_Unpack(recv_buffer, msg_size, &position, &ehp->nmeasure, 1, MPI_UINT64_T,
             CQ_MPI_COMM);

  size_t fname_size = 0;
  MPI_Unpack(recv_buffer, msg_size, &position, &fname_size, 1, MPI_UINT64_T,
             CQ_MPI_COMM);
  ehp->fname = (char*)malloc(fname_size);
  if (ehp->fname == NULL) {
    printf("%s [recv_exec_params]: malloc ehp->fname failed. Exiting\n",
           get_comm_source());
    exit(-10);
  }
  MPI_Unpack(recv_buffer, msg_size, &position, ehp->fname, fname_size, MPI_CHAR,
             CQ_MPI_COMM);

  const size_t qreg_size = sizeof(qubit) * ehp->nqubits;
  // NOTE: creg_size * expected_shots??
  const size_t creg_size = sizeof(cstate) * ehp->nmeasure;

  ehp->qreg = (qubit*)malloc(qreg_size);
  if (ehp->qreg == NULL) {
    printf("%s [recv_exec_params]: malloc ehp->qreg failed. Exiting\n",
           get_comm_source());
    exit(-10);
  }

  ehp->creg = (cstate*)malloc(creg_size);
  if (ehp->creg == NULL) {
    printf("%s [recv_exec_params]: malloc ehp->creg failed. Exiting\n",
           get_comm_source());
    exit(-10);
  }

  MPI_Unpack(recv_buffer, msg_size, &position, (char*)ehp->qreg, qreg_size,
             MPI_BYTE, CQ_MPI_COMM);
  MPI_Unpack(recv_buffer, msg_size, &position, (char*)ehp->creg, creg_size,
             MPI_BYTE, CQ_MPI_COMM);

  // NOTE: recv ehp->params left for another day... it's for pqkerns

  free(recv_buffer);

  printf("%s [recv_exec_params]: received.\n", get_comm_source());
  print_ehp(ehp);
}

void send_exec_params(cq_exec* ehp, int dest) {
  printf("%s [send_exec_params]: sending...\n", get_comm_source());
  print_ehp(ehp);

  const size_t bool_size = sizeof(bool);
  const size_t cq_status_size = sizeof(cq_status);
  int max_buffer_size = 0;
  int member_size = 0;
  MPI_Pack_size(bool_size, MPI_BYTE, CQ_MPI_COMM,
                &max_buffer_size);  // exec_init
  MPI_Pack_size(bool_size, MPI_BYTE, CQ_MPI_COMM,
                &member_size);  // complete
  max_buffer_size += member_size;
  MPI_Pack_size(bool_size, MPI_BYTE, CQ_MPI_COMM,
                &member_size);  // halt
  max_buffer_size += member_size;
  MPI_Pack_size(cq_status_size, MPI_BYTE, CQ_MPI_COMM,
                &member_size);  // status
  max_buffer_size += member_size;
  MPI_Pack_size(1, MPI_UINT64_T, CQ_MPI_COMM,
                &member_size);  // nqubits
  max_buffer_size += member_size;
  MPI_Pack_size(1, MPI_UINT64_T, CQ_MPI_COMM,
                &member_size);  // completed_shots
  max_buffer_size += member_size;
  MPI_Pack_size(1, MPI_UINT64_T, CQ_MPI_COMM,
                &member_size);  // expected_shots
  max_buffer_size += member_size;
  MPI_Pack_size(1, MPI_UINT64_T, CQ_MPI_COMM,
                &member_size);  // nmeasure
  max_buffer_size += member_size;

  // -------------------------------------------------------------------------
  // TODO: sending pthred stuff doesn't sound like a good idea...
  // sounds like UB
  //  const size_t pthread_mutex_size = sizeof(pthread_mutex_t);
  //  MPI_Pack_size(pthread_mutex_size, MPI_BYTE, CQ_MPI_COMM,
  //                &member_size);  // lock
  //  max_buffer_size += member_size;
  //
  //  const size_t pthread_cond_size = sizeof(pthread_cond_t);
  //  MPI_Pack_size(pthread_cond_size, MPI_BYTE, CQ_MPI_COMM,
  //                &member_size);  // cond_exec_complete
  //  max_buffer_size += member_size;
  // -------------------------------------------------------------------------

  size_t fname_size = 0;
  if (ehp->fname != NULL) {
    fname_size = strlen(ehp->fname) + 1;
  }
  MPI_Pack_size(1, MPI_UINT64_T, CQ_MPI_COMM,
                &member_size);  // fname_size
  max_buffer_size += member_size;
  MPI_Pack_size(fname_size, MPI_CHAR, CQ_MPI_COMM,
                &member_size);  // fname
  max_buffer_size += member_size;
  const size_t qreg_size = sizeof(qubit) * ehp->nqubits;
  MPI_Pack_size(qreg_size, MPI_BYTE, CQ_MPI_COMM,
                &member_size);  // qreg
  max_buffer_size += member_size;

  // NOTE: creg_size * expected_shots??
  const size_t creg_size = sizeof(cstate) * ehp->nmeasure;
  MPI_Pack_size(creg_size, MPI_BYTE, CQ_MPI_COMM,
                &member_size);  // creg
  max_buffer_size += member_size;

  // NOTE: sending ehp->params left for another day... it's for pqkerns

  char* send_buffer = malloc(max_buffer_size);
  if (send_buffer == NULL) {
    printf("Failed to allocate buffer for sending executor handle.\n");
    exit(-1);
  }
  int position;
  MPI_Pack(&ehp->exec_init, bool_size, MPI_BYTE, send_buffer, max_buffer_size,
           &position, CQ_MPI_COMM);
  MPI_Pack(&ehp->complete, bool_size, MPI_BYTE, send_buffer, max_buffer_size,
           &position, CQ_MPI_COMM);
  MPI_Pack(&ehp->halt, bool_size, MPI_BYTE, send_buffer, max_buffer_size,
           &position, CQ_MPI_COMM);
  MPI_Pack(&ehp->status, cq_status_size, MPI_BYTE, send_buffer, max_buffer_size,
           &position, CQ_MPI_COMM);

  MPI_Pack(&ehp->nqubits, 1, MPI_UINT64_T, send_buffer, max_buffer_size,
           &position, CQ_MPI_COMM);
  MPI_Pack(&ehp->completed_shots, 1, MPI_UINT64_T, send_buffer, max_buffer_size,
           &position, CQ_MPI_COMM);
  MPI_Pack(&ehp->expected_shots, 1, MPI_UINT64_T, send_buffer, max_buffer_size,
           &position, CQ_MPI_COMM);
  MPI_Pack(&ehp->nmeasure, 1, MPI_UINT64_T, send_buffer, max_buffer_size,
           &position, CQ_MPI_COMM);

  // NOTE: skip pthread stuff...

  MPI_Pack(&fname_size, 1, MPI_UINT64_T, send_buffer, max_buffer_size,
           &position, CQ_MPI_COMM);
  MPI_Pack(&ehp->fname, fname_size, MPI_CHAR, send_buffer, max_buffer_size,
           &position, CQ_MPI_COMM);

  MPI_Pack(ehp->qreg, qreg_size, MPI_BYTE, send_buffer, max_buffer_size,
           &position, CQ_MPI_COMM);
  MPI_Pack(&ehp->creg, creg_size, MPI_BYTE, send_buffer, max_buffer_size,
           &position, CQ_MPI_COMM);

  // NOTE: ehp->prams left for another day

  MPI_Ssend(&position, 1, MPI_INT, dest, CQ_MPI_COMMS_TAG, CQ_MPI_COMM);
  MPI_Ssend(send_buffer, position, MPI_PACKED, dest, CQ_MPI_COMMS_TAG,
            CQ_MPI_COMM);

  printf("%s [send_exec_params]: sent.\n", get_comm_source());
}

// ----------------------------------------------------------------------------
// Helpers
// ----------------------------------------------------------------------------

const char* get_comm_source() {
  if (mpi_env.rank == CQ_MPI_HOST_RANK) {
    return "Host:\t\t";
  } else if (mpi_env.rank == CQ_MPI_DEVICE_RANK) {
    return "Device:\t\t";
  } else {
    return "";
  }
}

int get_rank(void) {
  return mpi_env.rank;
}

const char* op_to_str(const enum ctrl_code OP) {
  switch (OP) {
    case CQ_CTRL_IDLE: {
      return "CQ_CTRL_IDLE";
      break;
    }
    case CQ_CTRL_ALLOC: {
      return "CQ_CTRL_ALLOC";
      break;
    }
    case CQ_CTRL_DEALLOC: {
      return "CQ_CTRL_DEALLOC";
      break;
    }
    case CQ_CTRL_INIT: {
      return "CQ_CTRL_INIT";
      break;
    }
    case CQ_CTRL_FINALISE: {
      return "CQ_CTRL_FINALISE";
      break;
    }
    case CQ_CTRL_WAIT: {
      return "CQ_CTRL_WAIT";
      break;
    }
    case CQ_CTRL_RUN_QKERNEL: {
      return "CQ_CTRL_RUN_QKERNEL";
      break;
    }
    case CQ_CTRL_RUN_PQKERNEL: {
      return "CQ_CTRL_RUN_PQKERNEL";
      break;
    }
    case CQ_CTRL_TEST: {
      return "CQ_CTRL_TEST";
      break;
    }
    case CQ_CTRL_ABORT: {
      return "CQ_CTRL_ABORT";
      break;
    }
    default: {
      break;
    }
  }
  return "";
}

void print_alloc_params(const device_alloc_params* params) {
  printf("%s alloc params: NQUBITS: %zu, qreg_idx: %zu, STATUS: %d\n",
         get_comm_source(), params->NQUBITS, params->qregistry_idx,
         params->status);
}

void print_ehp(const cq_exec* ehp) {
  if (ehp == NULL) {
    printf("ehp is NULL\n");
  }

  printf(
      "%s ehp details:\nexec_init: %d, complete: %d, halt: %d, STATUS: "
      "%d\nNQUBITS: "
      "%zu, completed_shots: %zu, expected_shots: %zu, NMEASURE: %zu\nfname: "
      "%s\nqreg:\n",
      get_comm_source(), ehp->exec_init, ehp->complete, ehp->halt, ehp->status,
      ehp->nqubits, ehp->completed_shots, ehp->expected_shots, ehp->nmeasure,
      ehp->fname);

  for (size_t i = 0; i < ehp->nqubits; ++i) {
    printf("qubit[%zu]: reg_idx: %zu, offset: %zu, N: %zu\n", i,
           ehp->qreg[i].registry_index, ehp->qreg[i].offset, ehp->qreg[i].N);
  }
  printf("\ncreg:\n");

  for (size_t i = 0; i < ehp->nmeasure; ++i) {
    printf("cstate[%zu]: %d\n", i, ehp->creg[i]);
  }
  printf("%s ehp details END\n\n", get_comm_source());
}
