#include "mpi_comms.h"

#include "datatypes.h"
#include "src/host-device/comms.h"
#include "src/host/opcodes.h"

#include <mpi.h>

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const MPI_Comm CQ_MPI_COMM = MPI_COMM_WORLD;

struct cq_mpi_env {
  int rank;
};

static cq_exec* executor_handles[__CQ_DEVICE_QUEUE_SIZE__];
// static size_t executor_id = 0;
static size_t global_exec_id_counter = -1;
static struct cq_mpi_env mpi_env = {.rank = -1};
static size_t num_active_executors = 0;

struct communicator {
  bool comm_busy;
  pthread_t device_comm_thread;
  pthread_cond_t cond_comm_busy;
  pthread_mutex_t comm_lock;
};

static struct communicator dev_comm = {0};
static unsigned int cq_mpi_verbosity = 0;

static void cq_log(const char* format, ...) {
  if (CQ_MPI_IMPL_DEBUG) {
    va_list(args);
    va_start(args, format);
    vprintf(format, args);
  }
}

void init_host_device_mpi(const unsigned int VERBOSITY) {
  cq_mpi_verbosity = VERBOSITY;
  if (VERBOSITY > 0) {
    cq_log("Initialising MPI.\n");
  }
  MPI_Init(NULL, NULL);
  MPI_Comm_rank(CQ_MPI_COMM, &mpi_env.rank);
  if (VERBOSITY > 0) {
    cq_log("Initialised MPI.\n");
  }

  if (mpi_env.rank != CQ_MPI_HOST_RANK) {
    device_init_comms(VERBOSITY);
    for (size_t i = 0; i < __CQ_DEVICE_QUEUE_SIZE__; ++i) {
      executor_handles[i] = NULL;
    }
  }
}

void finalise_host_device_mpi(const unsigned int VERBOSITY) {
  if (mpi_env.rank != CQ_MPI_HOST_RANK) {
    device_finalise_comms(VERBOSITY);
  }
  if (VERBOSITY > 0) {
    cq_log("%s Finalising MPI.\n", get_comm_source());
  }
  MPI_Finalize();
  if (VERBOSITY > 0) {
    cq_log("%s Finalised MPI.\n", get_comm_source());
  }
}

// void mpi_host_send_ctrl_op(const enum ctrl_code OP,
//                            struct ctrl_params* params) {
void mpi_host_send_ctrl_op(const enum ctrl_code OP, void* params) {
  const int device_rank = CQ_MPI_DEVICE_RANK;
  cq_log("%s [send_ctrl_op]: sending %s...\n", get_comm_source(),
         op_to_str(OP));
  MPI_Ssend(&OP, 1, MPI_INT, device_rank, CQ_MPI_COMMS_TAG, CQ_MPI_COMM);
  // send params based on OP
  host_comm_params(OP, params);
  cq_log("%s [send_ctrl_op]: sent %s\n", get_comm_source(), op_to_str(OP));
}

void mpi_host_wait_all_ops(void) {
  cq_log("%s [wait_all_ops]: waiting...\n", get_comm_source());
  mpi_host_send_ctrl_op(CQ_CTRL_WAIT, NULL);
  size_t num_ops;
  const int device_rank = CQ_MPI_DEVICE_RANK;
  MPI_Status status;
  MPI_Recv(&num_ops, 1, MPI_UINT64_T, device_rank, CQ_MPI_COMMS_TAG,
           CQ_MPI_COMM, &status);
  cq_log("%s [wait_all_ops]: all ops completed (num_ops: %zu)\n",
         get_comm_source(), num_ops);
}

void host_device_final_sync(void) {
  if (mpi_env.rank != CQ_MPI_HOST_RANK) {
    device_wait_comms();
  }
}

// ----------------------------------------------------------------------------
// Device Comm Ops
// ----------------------------------------------------------------------------

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

void device_init_comms(const unsigned int VERBOSITY) {
  if (VERBOSITY > 0) {
    cq_log(
        "%s [device_init_comms]: setting up on-device communication thread.\n",
        get_comm_source());
  }
  dev_ctrl.run_device = true;
  // cq_log("%s started listening...\n", get_comm_source());
  // device_listen();
  // cq_log("%s closing connection.\n", get_comm_source());
  // finalise_host_device_mpi(VERBOSITY);
  dev_comm.comm_busy = true;
  pthread_mutex_init(&dev_comm.comm_lock, NULL);
  pthread_cond_init(&dev_comm.cond_comm_busy, NULL);
  pthread_create(&dev_comm.device_comm_thread, NULL, &device_listen, NULL);
  if (VERBOSITY > 0) {
    cq_log("%s [device_init_comms]: on-device communication thread set up.\n",
           get_comm_source());
  }
}

void device_finalise_comms(const unsigned int VERBOSITY) {
  if (VERBOSITY > 0) {
    cq_log(
        "%s [device_finalise_comms]: finalising on-device communication "
        "thread.\n",
        get_comm_source());
  }

  pthread_join(dev_comm.device_comm_thread, NULL);
  dev_comm.comm_busy = false;
  pthread_cond_destroy(&dev_comm.cond_comm_busy);
  pthread_mutex_destroy(&dev_comm.comm_lock);
  if (VERBOSITY > 0) {
    cq_log(
        "%s [device_finalise_comms]: on-device communication thread closed.\n",
        get_comm_source());
  }
}

void* device_listen(void* args) {
  // run_device set to FALSE when OP == CQ_CTRL_FINALISE
  cq_log("%s started listening...\n", get_comm_source());
  while (dev_ctrl.run_device) {
    pthread_mutex_lock(&dev_comm.comm_lock);
    dev_comm.comm_busy = true;
    pthread_cond_signal(&dev_comm.cond_comm_busy);
    pthread_mutex_unlock(&dev_comm.comm_lock);

    enum ctrl_code OP;
    const int host_rank = CQ_MPI_HOST_RANK;
    MPI_Status status;
    cq_log("%s [device_listen]: receiving OP...\n", get_comm_source());
    MPI_Recv(&OP, 1, MPI_INT, host_rank, CQ_MPI_COMMS_TAG, CQ_MPI_COMM,
             &status);
    cq_log("%s [device_listen]: received %s\n", get_comm_source(),
           op_to_str(OP));

    device_dispatch_ctrl_op(OP);

    pthread_mutex_lock(&dev_comm.comm_lock);
    dev_comm.comm_busy = false;
    pthread_cond_signal(&dev_comm.cond_comm_busy);
    pthread_mutex_unlock(&dev_comm.comm_lock);
  }
  cq_log("%s closing connection.\n", get_comm_source());

  finalise_host_device_mpi(cq_mpi_verbosity);
  return NULL;
}

// optionally get params and run op
// by running op I mean modyfing the internal dev_ctrl fields
// and then the worker thread handles the rest.
void device_dispatch_ctrl_op(const enum ctrl_code OP) {
  switch (OP) {
    case CQ_CTRL_INIT: {
      initialise_device(cq_mpi_verbosity);
      insert_op(OP, &cq_mpi_verbosity);
      break;
    }
    case CQ_CTRL_FINALISE: {
      // don't need to insert op into worker.
      // we just wait until worker is done and cleanup.
      device_wait_all_ops();
      for (size_t i = 0; i < __CQ_DEVICE_QUEUE_SIZE__; ++i) {
        if (executor_handles[i] != NULL) {
          cq_log(
              "%s [dispatch][FINALISE]: executor handle with id: %zu is still "
              "active. Something went wrong!\n",
              get_comm_source(), i);
        }
      }
      finalise_device(cq_mpi_verbosity);
      break;
    }
    case CQ_CTRL_ALLOC: {
      // Alloc is blocking: we get params, run allocation and
      // send back the updated params.
      // Also, because it's blocking I don't need to worry about
      // params lifetime (from the worker perspective)
      const int host_rank = CQ_MPI_HOST_RANK;
      device_alloc_params params = {0};
      recv_alloc_params(&params, host_rank);
      cq_log("%s [dispatch][ALLOC]: inserting op\n", get_comm_source());
      insert_op(OP, &params);
      cq_log("%s [dispatch][ALLOC]: inserted op\n", get_comm_source());
      device_wait_all_ops();
      cq_log("%s [dispatch][ALLOC]: waited for finish\n", get_comm_source());
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
      if (num_active_executors >= __CQ_DEVICE_QUEUE_SIZE__) {
        cq_log(
            "%s [dispatch][RUN_QKERNEL]: You have oversubsribed the executor "
            "queue. We allow up to %d "
            "concurrent executors per device. Exiting",
            get_comm_source(), __CQ_DEVICE_QUEUE_SIZE__);
        exit(CQ_MPI_RUNTIME_ERROR);
      }
      const int host_rank = CQ_MPI_HOST_RANK;
      cq_exec* tmp_exec = NULL;
      recv_exec_params(&tmp_exec, host_rank);
      // NOTE: this can be done here to free old one
      // and allocate new one rather than in wait_exec
      // device_free_exec(&executor_handles[tmp_exec->id]);
      // actually if executor_handles[id] != NULL then it didn't finish and we
      // should wait here until we are done.
      //
      // NOTE: 2
      // Now the question is should we block?
      // 1. if we block here and wait for all ops to complete then
      // in situation if host submits N_exe > QUEUE_SIZE, we lock here
      // indefinitely
      // ex: a_qrun(...) x QUEUE_SIZE + 1 (and no wait_qrun...)
      // => deadlock
      // 2. alternative would be wait to set exec_queue_full flag
      // and wait until it is released
      // 3. we can just exit and fail! -- most reasonable:
      executor_handles[tmp_exec->id] = tmp_exec;
      insert_op(OP, executor_handles[tmp_exec->id]);
      ++num_active_executors;
      // recv_exec_params(&executor_handles[executor_id], host_rank);
      // insert_op(OP, executor_handles[executor_id]);
      break;
    }
    case CQ_CTRL_RUN_PQKERNEL: {
      // recv params
      // insert_op
      break;
    }
    case CQ_CTRL_WAIT_EXEC: {
      const int host_rank = CQ_MPI_HOST_RANK;
      const size_t executor_id = recv_exec_id(host_rank);
      if (executor_handles[executor_id] == NULL) {
        cq_log(
            "%s [dispatch][WAIT_EXEC]: device ehp is NULL. Returning. "
            "Exiting\n",
            get_comm_source());
        exit(CQ_MPI_RUNTIME_ERROR);
      }
      // NOTE: This is commented out as it can cause a deadlock
      // comms_exec_wait(executor_handles[executor_id]);
      device_wait_all_ops();
      send_exec_params(executor_handles[executor_id], host_rank);
      // NOTE: This can be done when allocating new in offload
      // i.e. clear old one and allocate new one
      device_free_exec(&executor_handles[executor_id]);
      --num_active_executors;
      if (num_active_executors < 0) {
        cq_log(
            "%s [dispatch][WAIT_EXEC]: The number of active executors is < 0! "
            "Should not happen. Exiting",
            get_comm_source());
        exit(CQ_MPI_RUNTIME_ERROR);
      }
      break;
    }
    case CQ_CTRL_SYNC_EXEC: {
      const int host_rank = CQ_MPI_HOST_RANK;
      const size_t executor_id = recv_exec_id(host_rank);
      if (executor_handles[executor_id] == NULL) {
        cq_log(
            "%s [dispatch][SYNC_EXEC]: device ehp is NULL. Returning. "
            "Exiting\n",
            get_comm_source());
        exit(CQ_MPI_RUNTIME_ERROR);
      }
      comms_exec_sync(executor_handles[executor_id]);
      cq_log("%s [dispatch][SYNC_EXEC]: executor synced\n", get_comm_source());
      send_exec_params(executor_handles[executor_id], host_rank);
      cq_log("%s [dispatch][SYNC_EXEC]: sent executor\n", get_comm_source());
      break;
    }
    case CQ_CTRL_WAIT: {
      // wait for worker and send info to the host
      // (which is blocked and waiting).
      size_t num_ops = device_wait_all_ops();
      const int host_rank = CQ_MPI_HOST_RANK;
      cq_log("%s [dispatch]: sending num ops: %zu...\n", get_comm_source(),
             num_ops);
      MPI_Ssend(&num_ops, 1, MPI_UINT64_T, host_rank, CQ_MPI_COMMS_TAG,
                CQ_MPI_COMM);
      cq_log("%s [dispatch]: sent num ops.\n", get_comm_source());
      break;
    }
    case CQ_CTRL_ABORT: {
      const int host_rank = CQ_MPI_HOST_RANK;
      const size_t executor_id = recv_exec_id(host_rank);
      if (executor_handles[executor_id] == NULL) {
        cq_log(
            "%s [dispatch][ABORT]: device ehp is NULL. Returning. "
            "Exiting\n",
            get_comm_source());
        exit(CQ_MPI_RUNTIME_ERROR);
      }
      comms_exec_halt(executor_handles[executor_id]);
      cq_log("%s [dispatch][ABORT]: executor halted\n", get_comm_source());
      break;
    }
    case CQ_CTRL_IDLE: {
      break;
    }
    case CQ_CTRL_TEST: {
      cq_log("%s [dispatch][TEST]: called test op\n", get_comm_source());
      break;
    }
    default: {
      break;
    }
  }
}

// NOTE: aka host_wait_all_ops from comm.c
// really what I want to use is host_wait_all_ops implementation but it needs
// renaming and wrapping into function called host_send_ctrl_op and depending
// on either MPI or pthread implementation use this in different ctx.
size_t device_wait_all_ops(void) {
  pthread_mutex_lock(&dev_ctrl.device_lock);
  while (dev_ctrl.num_ops > 0 || dev_ctrl.device_busy) {
    pthread_cond_wait(&dev_ctrl.cond_device_busy, &dev_ctrl.device_lock);
  }

  pthread_mutex_unlock(&dev_ctrl.device_lock);
  return dev_ctrl.num_ops;
}

void device_wait_comms(void) {
  pthread_mutex_lock(&dev_comm.comm_lock);
  while (dev_comm.comm_busy) {
    pthread_cond_wait(&dev_comm.cond_comm_busy, &dev_comm.comm_lock);
  }
  pthread_mutex_unlock(&dev_comm.comm_lock);
}

// for those below I probably I want comms_exec_sync, comms_exec_wait,
// comms_exec_halt
// NOTE: really what I want to use is host_sync_exec
// implemenetation but it needs renaming and wrapping into function called
// host_send_ctrl_op and depending on either MPI or pthread implementation use
// this in different ctx.
size_t comms_exec_sync(cq_exec* const ehp) {
  size_t completed_shots = 0;
  pthread_mutex_lock(&(ehp->lock));
  completed_shots = ehp->completed_shots;
  pthread_mutex_unlock(&(ehp->lock));
  return completed_shots;
}

// NOTE:
// really what I want to use is host_wait_exec implemenetation but it needs
// renaming and wrapping into function called host_send_ctrl_op and depending
// on either MPI or pthread implementation use this in different ctx.
size_t comms_exec_wait(cq_exec* const ehp) {
  cq_log("%s [comms_exec_wait]: started wait...\n", get_comm_source());
  pthread_mutex_lock(&(ehp->lock));
  cq_log("%s [comms_exec_wait]: locked mutex\n", get_comm_source());
  while (!ehp->complete) {
    cq_log("%s [comms_exec_wait]: started loop\n", get_comm_source());
    pthread_cond_wait(&(ehp->cond_exec_complete), &(ehp->lock));
    cq_log("%s [comms_exec_wait]: ending loop\n", get_comm_source());
  }
  cq_log("%s [comms_exec_wait]: done while loop\n", get_comm_source());
  pthread_mutex_unlock(&(ehp->lock));
  cq_log("%s [comms_exec_wait]: unlocked mutex\n", get_comm_source());
  cq_log("%s [comms_exec_wait]: done waiting\n", get_comm_source());
  return ehp->completed_shots;
}

// NOTE:
// really what I want to use is host_request_halt implemenetation but it needs
// renaming and wrapping into function called host_send_ctrl_op and depending
// on either MPI or pthread implementation use this in different ctx.
void comms_exec_halt(cq_exec* const ehp) {
  pthread_mutex_lock(&(ehp->lock));
  ehp->halt = true;
  pthread_mutex_unlock(&(ehp->lock));
  return;
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
      send_exec_id(((cq_exec*)params)->id, device_rank);
      recv_exec_params(&params, device_rank);
      break;
    }
    case CQ_CTRL_SYNC_EXEC: {
      const int device_rank = CQ_MPI_DEVICE_RANK;
      send_exec_id(((cq_exec*)params)->id, device_rank);
      recv_exec_params(&params, device_rank);
      break;
    }
    case CQ_CTRL_ABORT: {
      const int device_rank = CQ_MPI_DEVICE_RANK;
      send_exec_id(((cq_exec*)params)->id, device_rank);
      break;
    }
    default: {
      break;
    }
  }
}

void recv_alloc_params(device_alloc_params* params, int src) {
  cq_log("%s [recv_alloc_params]: receiving...\n", get_comm_source());

  const size_t params_size = sizeof(device_alloc_params);
  MPI_Status status;

  MPI_Recv(params, params_size, MPI_BYTE, src, CQ_MPI_COMMS_TAG, CQ_MPI_COMM,
           &status);

  cq_log("%s [recv_alloc_params]: received.\n", get_comm_source());
  print_alloc_params(params);
}

void send_alloc_params(const device_alloc_params* params, int dest) {
  cq_log("%s [send_alloc_params]: sending...\n", get_comm_source());
  const size_t params_size = sizeof(device_alloc_params);
  print_alloc_params(params);
  MPI_Ssend((char*)params, params_size, MPI_BYTE, dest, CQ_MPI_COMMS_TAG,
            CQ_MPI_COMM);
  cq_log("%s [send_alloc_params]: sent.\n", get_comm_source());
}

size_t recv_exec_id(const int src) {
  cq_log("%s [recv_exec_id]: receiving...\n", get_comm_source());
  size_t id = -1;
  MPI_Status status;
  MPI_Recv(&id, 1, MPI_UINT64_T, src, CQ_MPI_COMMS_TAG, CQ_MPI_COMM, &status);
  cq_log("%s [recv_exec_id]: received id: %zu\n", get_comm_source(), id);
  return id;
}

void send_exec_id(const size_t id, const int dest) {
  cq_log("%s [send_exec_id]: sending id: %zu...\n", get_comm_source(), id);
  MPI_Ssend(&id, 1, MPI_UINT64_T, dest, CQ_MPI_COMMS_TAG, CQ_MPI_COMM);
  cq_log("%s [send_exec_id]: sent\n", get_comm_source());
}

void recv_exec_params(cq_exec** ehp, int src) {
  // TODO: do validation
  cq_log("%s [recv_exec_params]: receiving...\n", get_comm_source());
  int msg_size;
  MPI_Status status;
  MPI_Recv(&msg_size, 1, MPI_INT, src, CQ_MPI_COMMS_TAG, CQ_MPI_COMM, &status);
  void* recv_buffer = malloc(msg_size);

  // reserve space for all the date + currently unused members
  if (*ehp == NULL) {
    cq_log("%s [recv_exec_params]: *ehp is NULL. Allocating on device.\n",
           get_comm_source());

    // ehp = (cq_exec*)malloc(msg_size + sizeof(pthread_mutex_t) +
    //                        sizeof(pthread_cond_t) + sizeof(void*));
    *ehp = (cq_exec*)malloc(sizeof(cq_exec));
    pthread_mutex_init(&(*ehp)->lock, NULL);
    pthread_cond_init(&(*ehp)->cond_exec_complete, NULL);
  } else {
    cq_log(
        "%s [recv_exec_params]: *ehp is already allocated. So I'm on host.\n",
        get_comm_source());
  }
  pthread_mutex_lock(&(*ehp)->lock);

  if (recv_buffer == NULL || *ehp == NULL) {
    cq_log("%s [recv_exec_params]: malloc failed. Exiting\n",
           get_comm_source());
    exit(CQ_MPI_MALLOC_ERROR);
  }

  MPI_Recv(recv_buffer, msg_size, MPI_PACKED, src, CQ_MPI_COMMS_TAG,
           CQ_MPI_COMM, &status);

  const size_t bool_size = sizeof(bool);
  const size_t cq_status_size = sizeof(cq_status);
  // unpack
  int position = 0;
  MPI_Unpack(recv_buffer, msg_size, &position, &(*ehp)->id, 1, MPI_UINT64_T,
             CQ_MPI_COMM);
  MPI_Unpack(recv_buffer, msg_size, &position, &(*ehp)->exec_init, bool_size,
             MPI_BYTE, CQ_MPI_COMM);
  MPI_Unpack(recv_buffer, msg_size, &position, &(*ehp)->complete, bool_size,
             MPI_BYTE, CQ_MPI_COMM);
  MPI_Unpack(recv_buffer, msg_size, &position, &(*ehp)->halt, bool_size,
             MPI_BYTE, CQ_MPI_COMM);
  MPI_Unpack(recv_buffer, msg_size, &position, &(*ehp)->status, cq_status_size,
             MPI_BYTE, CQ_MPI_COMM);
  MPI_Unpack(recv_buffer, msg_size, &position, &(*ehp)->nqubits, 1,
             MPI_UINT64_T, CQ_MPI_COMM);
  MPI_Unpack(recv_buffer, msg_size, &position, &(*ehp)->completed_shots, 1,
             MPI_UINT64_T, CQ_MPI_COMM);
  MPI_Unpack(recv_buffer, msg_size, &position, &(*ehp)->expected_shots, 1,
             MPI_UINT64_T, CQ_MPI_COMM);
  MPI_Unpack(recv_buffer, msg_size, &position, &(*ehp)->nmeasure, 1,
             MPI_UINT64_T, CQ_MPI_COMM);

  size_t fname_size = 0;
  MPI_Unpack(recv_buffer, msg_size, &position, &fname_size, 1, MPI_UINT64_T,
             CQ_MPI_COMM);
  fname_size *= sizeof(char);

  const size_t qreg_size = sizeof(qubit) * (*ehp)->nqubits;
  // NOTE: creg_size * expected_shots?? or completed_shots?
  // host should get completed, device should get expected
  // and similarly when sending
  size_t num_shots = 0;
  if (mpi_env.rank == CQ_MPI_HOST_RANK) {
    num_shots = (*ehp)->completed_shots;
  } else {
    num_shots = (*ehp)->expected_shots;
  }
  const size_t creg_size = sizeof(cstate) * (*ehp)->nmeasure * num_shots;

  // when on host the resources are already allocated!
  if (mpi_env.rank != CQ_MPI_HOST_RANK) {
    (*ehp)->fname = (char*)malloc(fname_size);
    if ((*ehp)->fname == NULL) {
      cq_log("%s [recv_exec_params]: malloc *ehp->fname failed. Exiting\n",
             get_comm_source());
      exit(CQ_MPI_MALLOC_ERROR);
    }

    (*ehp)->qreg = (qubit*)malloc(qreg_size);
    if ((*ehp)->qreg == NULL) {
      cq_log("%s [recv_exec_params]: malloc *ehp->qreg failed. Exiting\n",
             get_comm_source());
      exit(CQ_MPI_MALLOC_ERROR);
    }

    (*ehp)->creg = (cstate*)malloc(creg_size);
    if ((*ehp)->creg == NULL) {
      cq_log("%s [recv_exec_params]: malloc *ehp->creg failed. Exiting\n",
             get_comm_source());
      exit(CQ_MPI_MALLOC_ERROR);
    }
  }

  MPI_Unpack(recv_buffer, msg_size, &position, (*ehp)->fname, fname_size,
             MPI_CHAR, CQ_MPI_COMM);
  MPI_Unpack(recv_buffer, msg_size, &position, (char*)(*ehp)->qreg, qreg_size,
             MPI_BYTE, CQ_MPI_COMM);
  MPI_Unpack(recv_buffer, msg_size, &position, (char*)(*ehp)->creg, creg_size,
             MPI_BYTE, CQ_MPI_COMM);

  // NOTE: recv ehp->params left for another day... it's for pqkerns

  free(recv_buffer);

  cq_log("%s [recv_exec_params]: received.\n", get_comm_source());
  print_ehp(*ehp);
  pthread_mutex_unlock(&(*ehp)->lock);
}

void send_exec_params(cq_exec* ehp, int dest) {
  pthread_mutex_lock(&ehp->lock);
  cq_log("%s [send_exec_params]: sending...\n", get_comm_source());
  print_ehp(ehp);

  const size_t bool_size = sizeof(bool);
  const size_t cq_status_size = sizeof(cq_status);
  int max_buffer_size = 0;
  int member_size = 0;
  MPI_Pack_size(1, MPI_UINT64_T, CQ_MPI_COMM,
                &max_buffer_size);  // id
  MPI_Pack_size(bool_size, MPI_BYTE, CQ_MPI_COMM,
                &member_size);  // exec_init
  max_buffer_size += member_size;
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

  // NOTE: creg_size * expected_shots?? or completed_shots?
  // host should send expected, device should send completed
  size_t num_shots = 0;
  if (mpi_env.rank == CQ_MPI_HOST_RANK) {
    num_shots = ehp->expected_shots;
  } else {
    num_shots = ehp->completed_shots;
  }
  const size_t creg_size = sizeof(cstate) * ehp->nmeasure * num_shots;

  MPI_Pack_size(creg_size, MPI_BYTE, CQ_MPI_COMM,
                &member_size);  // creg
  max_buffer_size += member_size;

  // NOTE: sending ehp->params left for another day... it's for pqkerns

  void* send_buffer = malloc(max_buffer_size);
  if (send_buffer == NULL) {
    cq_log("Failed to allocate buffer for sending executor handle. Exiting\n");
    exit(CQ_MPI_MALLOC_ERROR);
  }
  int position = 0;
  MPI_Pack(&ehp->id, 1, MPI_UINT64_T, send_buffer, max_buffer_size, &position,
           CQ_MPI_COMM);
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
  MPI_Pack(ehp->fname, fname_size, MPI_CHAR, send_buffer, max_buffer_size,
           &position, CQ_MPI_COMM);

  MPI_Pack(ehp->qreg, qreg_size, MPI_BYTE, send_buffer, max_buffer_size,
           &position, CQ_MPI_COMM);
  MPI_Pack(ehp->creg, creg_size, MPI_BYTE, send_buffer, max_buffer_size,
           &position, CQ_MPI_COMM);

  // NOTE: ehp->prams left for another day

  MPI_Ssend(&position, 1, MPI_INT, dest, CQ_MPI_COMMS_TAG, CQ_MPI_COMM);

  MPI_Ssend(send_buffer, position, MPI_PACKED, dest, CQ_MPI_COMMS_TAG,
            CQ_MPI_COMM);

  free(send_buffer);
  cq_log("%s [send_exec_params]: sent.\n", get_comm_source());
  pthread_mutex_unlock(&ehp->lock);
}

void device_free_exec(cq_exec** ehp) {
  cq_log("%s [device_free_exec]: freeing the executor\n", get_comm_source());
  if (ehp == NULL) {
    cq_log("%s [device_free_exec]: ehp is NULL\n", get_comm_source());
    return;
  }

  if (*ehp == NULL) {
    cq_log("%s [device_free_exec]: *ehp is NULL\n", get_comm_source());
    return;
  }

  pthread_mutex_lock(&(*ehp)->lock);
  if ((*ehp)->fname != NULL) {
    free((*ehp)->fname);
    (*ehp)->fname = NULL;
  }

  if ((*ehp)->qreg != NULL) {
    free((*ehp)->qreg);
    (*ehp)->qreg = NULL;
  }

  if ((*ehp)->creg != NULL) {
    free((*ehp)->creg);
    (*ehp)->creg = NULL;
  }

  if ((*ehp)->params != NULL) {
    free((*ehp)->params);
    (*ehp)->params = NULL;
  }

  pthread_mutex_unlock(&(*ehp)->lock);
  pthread_mutex_destroy(&(*ehp)->lock);
  pthread_cond_destroy(&(*ehp)->cond_exec_complete);
  if ((*ehp) != NULL) {
    free((*ehp));
    (*ehp) = NULL;
  }

  cq_log("%s [device_free_exec]: freed the executor\n", get_comm_source());
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
    case CQ_CTRL_WAIT_EXEC: {
      return "CQ_CTRL_WAIT_EXEC";
      break;
    }
    case CQ_CTRL_SYNC_EXEC: {
      return "CQ_CTRL_SYNC_EXEC";
      break;
    }
    default: {
      break;
    }
  }
  return "";
}

void print_alloc_params(const device_alloc_params* params) {
  cq_log("%s alloc params: NQUBITS: %zu, qreg_idx: %zu, STATUS: %d\n",
         get_comm_source(), params->NQUBITS, params->qregistry_idx,
         params->status);
}

void print_ehp(const cq_exec* ehp) {
  if (ehp == NULL) {
    cq_log("ehp is NULL\n");
  }

  cq_log(
      "%s ehp details:\nid: %zu, exec_init: %d, complete: %d, halt: %d, "
      "STATUS: "
      "%d\nNQUBITS: "
      "%zu, completed_shots: %zu, expected_shots: %zu, NMEASURE: %zu\nfname: "
      "%s\nqreg:\n",
      get_comm_source(), ehp->id, ehp->exec_init, ehp->complete, ehp->halt,
      ehp->status, ehp->nqubits, ehp->completed_shots, ehp->expected_shots,
      ehp->nmeasure, ehp->fname);

  for (size_t i = 0; i < ehp->nqubits; ++i) {
    cq_log("qubit[%zu]: reg_idx: %zu, offset: %zu, N: %zu\n", i,
           ehp->qreg[i].registry_index, ehp->qreg[i].offset, ehp->qreg[i].N);
  }
  cq_log("\ncreg:\n");

  for (size_t i = 0; i < ehp->nmeasure * ehp->expected_shots; ++i) {
    cq_log("cstate[%zu]: %d\n", i, ehp->creg[i]);
  }
  cq_log("%s ehp details END\n\n", get_comm_source());
}

size_t assign_exec_id(void) {
  //  ++executor_id;
  // return executor_id;
  ++global_exec_id_counter;
  global_exec_id_counter %= __CQ_DEVICE_QUEUE_SIZE__;
  return global_exec_id_counter;
}
