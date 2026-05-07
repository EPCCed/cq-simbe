#include "mpi_comms.h"

#include "src/host-device/comms.h"
#include "src/host/opcodes.h"

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
    printf("%s [device_listen]: recieving OP...\n", get_comm_source());
    MPI_Recv(&OP, 1, MPI_INT, host_rank, CQ_MPI_COMMS_TAG, CQ_MPI_COMM,
             &status);
    printf("%s [device_listen]: recieved %s\n", get_comm_source(),
           op_to_str(OP));

    // optionally get params and run op
    // by running op I mean modyfing the internal dev_ctrl fields
    // and then the worker thread handles the rest.
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

  // unsigned int* ctrl_params = (unsigned int*)malloc(sizeof(unsigned int));
  //*ctrl_params = 1;
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
      // I think this is the correct way to do things
      // because alloc is blocking
      // also because of it being blocking I don't need
      // to worry about params lifetime (from the worker perspecitve)
      // recv params
      // device_alloc_params params;
      // recv_alloc_params(device_alloc_params* params, int src);
      // insert_op(OP, params);
      // wait?
      // device_wait_all_ops();
      // send updated params
      // send_alloc_params(device_alloc_params* params, int dest);
      const int host_rank = CQ_MPI_HOST_RANK;
      device_alloc_params params = {0};
      recv_alloc_params(&params, host_rank);
      insert_op(OP, &params);
      device_wait_all_ops();
      send_alloc_params(&params, host_rank);
      break;
    }
    case CQ_CTRL_DEALLOC: {
      // I think this is the correct way to do things
      // because dealloc is blocking
      // recv params
      // insert_op
      // wait?
      // send updated params
      const int host_rank = CQ_MPI_HOST_RANK;
      device_alloc_params params = {0};
      recv_alloc_params(&params, host_rank);
      insert_op(OP, &params);
      device_wait_all_ops();
      send_alloc_params(&params, host_rank);
      break;
    }
    case CQ_CTRL_RUN_QKERNEL: {
      // recv params
      // insert_op
      break;
    }
    case CQ_CTRL_RUN_PQKERNEL: {
      // recv params
      // insert_op
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
    default: {
      break;
    }
  }
}

void recv_alloc_params(device_alloc_params* params, int src) {
  printf("%s [recv_alloc_params]: recieving...\n", get_comm_source());

  const size_t params_size = sizeof(device_alloc_params);
  //  void* recv_buffer = malloc(params_size);
  //  if (recv_buffer == NULL) {
  //    exit(-1);
  //  }

  MPI_Status status;

  MPI_Recv((char*)params, params_size, MPI_BYTE, src, CQ_MPI_COMMS_TAG,
           CQ_MPI_COMM, &status);

  // memcpy((char*)params, recv_buffer, params_size);
  //  free(recv_buffer);

  printf("%s [recv_alloc_params]: recieved.\n", get_comm_source());
  print_alloc_params(params);
}

void send_alloc_params(const device_alloc_params* params, int dest) {
  const size_t params_size = sizeof(device_alloc_params);
  printf("%s [send_alloc_params]: sending...\n", get_comm_source());
  print_alloc_params(params);
  MPI_Ssend((char*)params, params_size, MPI_BYTE, dest, CQ_MPI_COMMS_TAG,
            CQ_MPI_COMM);
  printf("%s [send_alloc_params]: sent.\n", get_comm_source());
}

void recv_exec_params(device_alloc_params* params, int src) {}
void send_exec_params(device_alloc_params* params, int dest) {}

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
