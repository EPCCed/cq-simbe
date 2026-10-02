#include "comms_mpi.h"
#include "comms_core.h"
#include "kernel_utils.h"
#include "quest/include/config.h"
#include "quest/include/environment.h"
#if QUEST_COMPILE_MPI
#include "quest/include/experimental.h"
#if !QUEST_COMPILE_SUBCOMM
#error "MPI QuEST requires SUBCOMM support"
#endif
#endif
#include <limits.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* The listener always broadcasts within a device group, including the
   one-device case with a serial QuEST build. QuEST duplicates its input. */
#define CQ_MPI_DEVICE_MASTER_RANK 0
#define CQ_MPI_WORLD_COMMS_TAG 0
#define CQ_MPI_RUNTIME_ERROR -4
#define CQ_MPI_MALLOC_ERROR -5
static MPI_Comm CQ_MPI_COMM_WORLD = MPI_COMM_NULL;
static MPI_Comm CQ_MPI_SPLIT_COMM = MPI_COMM_NULL;
static MPI_Comm registration_comm = MPI_COMM_NULL;
static MPI_Comm worker_control_comm = MPI_COMM_NULL;
static bool owns_mpi_comm;
static int CQ_MPI_HOST_RANK;
static int CQ_MPI_DEVICE_RANK;
static struct {
  bool is_init;
  int rank, subcomm_rank, world_size;
  unsigned int verbosity;
} mpi_env = {0, -1, -1, 0, 0};
static cq_exec *executor_handles[__CQ_DEVICE_QUEUE_SIZE__];
static cq_exec *host_handles[__CQ_DEVICE_QUEUE_SIZE__];
static size_t num_active_executors;
static char debug_worker_name[64];
static struct {
  pthread_t device_comm_thread;
  pthread_mutex_t comm_lock;
  pthread_cond_t cond_comm_busy;
  bool sync_init, started, release, cancel;
  cq_status status;
} dev_comm;

static void cq_log(const char *format, ...) {
#ifdef DEBUG_MODE
  va_list args;
  va_start(args, format);
  vprintf(format, args);
  va_end(args);
#endif
}
static bool cq_validate_mpi_rank(int rank) {
  return rank >= 0 && rank < mpi_env.world_size;
}
static bool is_serial(void) { return mpi_env.world_size == 1; }
bool is_device(void) { return mpi_env.is_init && !is_serial() && mpi_env.rank != 0; }
static int agree_ready(int ready) {
  int result = 0;
  if (MPI_Allreduce(&ready, &result, 1, MPI_INT, MPI_MIN, registration_comm) != MPI_SUCCESS)
    return 0;
  return result;
}
static void release_listener(bool cancel) {
  if (!dev_comm.started) return;
  pthread_mutex_lock(&dev_comm.comm_lock);
  dev_comm.cancel = cancel;
  dev_comm.release = true;
  pthread_cond_broadcast(&dev_comm.cond_comm_busy);
  pthread_mutex_unlock(&dev_comm.comm_lock);
}
void device_finalise_comms(const unsigned int verbosity) {
  if (dev_comm.started) {
    pthread_join(dev_comm.device_comm_thread, NULL);
    dev_comm.started = false;
  }
  if (dev_comm.sync_init) {
    pthread_cond_destroy(&dev_comm.cond_comm_busy);
    pthread_mutex_destroy(&dev_comm.comm_lock);
    dev_comm.sync_init = false;
  }
}
static cq_status cleanup_mpi(void) {
  cq_status status = CQ_SUCCESS;
  if (worker_control_comm != MPI_COMM_NULL) MPI_Comm_free(&worker_control_comm);
  if (CQ_MPI_SPLIT_COMM != MPI_COMM_NULL) MPI_Comm_free(&CQ_MPI_SPLIT_COMM);
  if (registration_comm != MPI_COMM_NULL) MPI_Comm_free(&registration_comm);
  if (CQ_MPI_COMM_WORLD != MPI_COMM_NULL) MPI_Comm_free(&CQ_MPI_COMM_WORLD);
  mpi_env.is_init = false;
  if (owns_mpi_comm) {
    if (MPI_Finalize() != MPI_SUCCESS) status = CQ_ERROR;
    owns_mpi_comm = false;
  }
  return status;
}

static cq_status start_transport(MPI_Comm supplied, bool custom, unsigned int verbosity) {
  if (mpi_env.is_init) return CQ_WARNING;
  int initialized = 0, finalized = 0, provided = MPI_THREAD_SINGLE;
  if (MPI_Finalized(&finalized) != MPI_SUCCESS || finalized ||
      MPI_Initialized(&initialized) != MPI_SUCCESS) return CQ_ERROR;
  if (custom && (!initialized || supplied == MPI_COMM_NULL)) return CQ_ERROR;
  if (initialized) {
    /* Before duplicating anything or starting a thread, use the caller. */
    if (MPI_Query_thread(&provided) != MPI_SUCCESS) return CQ_ERROR;
  } else {
    if (MPI_Init_thread(NULL, NULL, MPI_THREAD_MULTIPLE, &provided) != MPI_SUCCESS)
      return CQ_ERROR;
    owns_mpi_comm = true;
  }
  MPI_Comm input = custom ? supplied : MPI_COMM_WORLD;
  int intercommunicator = 0;
  if (MPI_Comm_test_inter(input, &intercommunicator) != MPI_SUCCESS || intercommunicator) {
    if (owns_mpi_comm) { MPI_Finalize(); owns_mpi_comm = false; }
    return CQ_ERROR;
  }
  int local_ready = provided >= MPI_THREAD_MULTIPLE && !isQuESTEnvInit();
  int all_ready = 0;
  MPI_Allreduce(&local_ready, &all_ready, 1, MPI_INT, MPI_MIN, input);
  if (!all_ready) {
    if (owns_mpi_comm) { MPI_Finalize(); owns_mpi_comm = false; }
    return CQ_ERROR;
  }
  MPI_Comm_dup(input, &CQ_MPI_COMM_WORLD);
  MPI_Comm_dup(input, &registration_comm);
  MPI_Comm_size(input, &mpi_env.world_size);
  MPI_Comm_rank(input, &mpi_env.rank);
  mpi_env.verbosity = verbosity;
  int devices = mpi_env.world_size - 1;
#if QUEST_COMPILE_MPI
  local_ready = is_serial() || (devices > 0 && !(devices & (devices - 1)));
#else
  local_ready = is_serial() || devices == 1;
#endif
  if (!agree_ready(local_ready)) { cleanup_mpi(); return CQ_ERROR; }
  CQ_MPI_DEVICE_RANK = is_serial() ? 0 : 1;
  bool quantum = is_serial() || mpi_env.rank != 0;
  MPI_Comm_split(input, quantum ? 1 : MPI_UNDEFINED, mpi_env.rank, &CQ_MPI_SPLIT_COMM);
  if (quantum) {
    MPI_Comm_rank(CQ_MPI_SPLIT_COMM, &mpi_env.subcomm_rank);
    MPI_Comm_dup(CQ_MPI_SPLIT_COMM, &worker_control_comm);
  }
  /* Every rank joins readiness before a worker can enter QuEST collectives. */
  local_ready = !quantum || init_device_controls(verbosity) == CQ_SUCCESS;
  if (!agree_ready(local_ready)) {
    if (dev_ctrl.worker_started) { stop_device(); finalise_device_controls(verbosity); }
    cleanup_mpi();
    return CQ_ERROR;
  }
  local_ready = 1;
  if (quantum && !is_serial()) {
    memset(&dev_comm, 0, sizeof(dev_comm));
    dev_comm.status = CQ_SUCCESS;
    if (pthread_mutex_init(&dev_comm.comm_lock, NULL) != 0) local_ready = 0;
    else if (pthread_cond_init(&dev_comm.cond_comm_busy, NULL) != 0) {
      pthread_mutex_destroy(&dev_comm.comm_lock); local_ready = 0;
    } else {
      dev_comm.sync_init = true;
      if (pthread_create(&dev_comm.device_comm_thread, NULL, device_listen, NULL) != 0)
        local_ready = 0;
      else dev_comm.started = true;
    }
  }
  if (!agree_ready(local_ready)) {
    release_listener(true);
    device_finalise_comms(verbosity);
    if (quantum) {
      stop_device(); finalise_device_controls(verbosity);
    }
    cleanup_mpi(); return CQ_ERROR;
  }
  cq_status init_status = CQ_SUCCESS;
  if (quantum) {
    insert_op(CQ_CTRL_INIT, &verbosity);
    device_wait_all_ops();
    init_status = dev_ctrl.lifecycle_status;
  }
  if (!agree_ready(init_status == CQ_SUCCESS)) {
    release_listener(true);
    device_finalise_comms(verbosity);
    if (quantum) {
      if (init_status == CQ_SUCCESS) {
        insert_op(CQ_CTRL_FINALISE, &verbosity);
        device_wait_all_ops();
      }
      stop_device(); finalise_device_controls(verbosity);
    }
    cleanup_mpi(); return CQ_ERROR;
  }
  mpi_env.is_init = true;
  release_listener(false);
  return CQ_SUCCESS;
}
int initialise_device(unsigned int verbosity) {
  return start_transport(MPI_COMM_WORLD, false, verbosity);
}
int initialise_device_with_custom_mpi_comm(MPI_Comm comm, unsigned int verbosity) {
  return start_transport(comm, true, verbosity);
}
cq_status init_quest_env(void) {
  if (isQuESTEnvInit() || CQ_MPI_SPLIT_COMM == MPI_COMM_NULL) return CQ_ERROR;
#if QUEST_COMPILE_MPI
  initCustomMpiCommQuESTEnv(CQ_MPI_SPLIT_COMM, -1, -1);
#else
  initQuESTEnv();
#endif
  return isQuESTEnvInit() ? CQ_SUCCESS : CQ_ERROR;
}
cq_status finish_quest_env(void) { return CQ_SUCCESS; }

size_t host_send_ctrl_op(enum ctrl_code op, void *params) {
  if (is_serial()) {
    size_t result = serial_host_send_ctrl_op(op, params);
    if (op == CQ_CTRL_WAIT_EXEC) {
      cq_exec *exec = params;
      if (exec->id < __CQ_DEVICE_QUEUE_SIZE__ && host_handles[exec->id] == exec)
        host_handles[exec->id] = NULL;
    }
    return result;
  }
  int code = op;
  MPI_Ssend(&code, 1, MPI_INT, CQ_MPI_DEVICE_RANK, CQ_MPI_WORLD_COMMS_TAG, CQ_MPI_COMM_WORLD);
  host_comm_params(op, params);
  return 0;
}
size_t host_wait_all_ops(void) {
  if (is_serial()) return device_wait_all_ops();
  host_send_ctrl_op(CQ_CTRL_WAIT, NULL);
  uint64_t pending;
  MPI_Recv(&pending, 1, MPI_UINT64_T, CQ_MPI_DEVICE_RANK,
           CQ_MPI_WORLD_COMMS_TAG, CQ_MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  return pending;
}
void host_device_sync_comms(void) { /* Shutdown uses explicit listener joins. */ }
void device_wait_comms(void) { device_finalise_comms(mpi_env.verbosity); }

bool exec_is_live(const cq_exec *exec) {
  if (!exec) return false;
  for (size_t id=0; id<__CQ_DEVICE_QUEUE_SIZE__; ++id)
    if (host_handles[id] == exec) return true;
  return false;
}
cq_status submit_exec(enum ctrl_code op, cq_exec *exec) {
  if (!mpi_env.is_init || exec_is_live(exec)) return CQ_ERROR;
  size_t id;
  for (id = 0; id < __CQ_DEVICE_QUEUE_SIZE__; ++id) if (!host_handles[id]) break;
  if (id == __CQ_DEVICE_QUEUE_SIZE__) return CQ_ERROR;
  exec->id = id;
  host_handles[id] = exec;
  host_send_ctrl_op(op, exec);
  return CQ_SUCCESS;
}
int finalise_device(unsigned int verbosity) {
  if (!mpi_env.is_init) return CQ_WARNING;
  cq_status status = CQ_SUCCESS;
  if (is_serial() || mpi_env.rank == 0) {
    for (size_t id = 0; id < __CQ_DEVICE_QUEUE_SIZE__; ++id) {
      cq_exec *exec = host_handles[id];
      if (exec) {
        host_send_ctrl_op(CQ_CTRL_WAIT_EXEC, exec);
        if (exec->status != CQ_SUCCESS) status = CQ_ERROR;
        finalise_exec_handle(exec);
      }
    }
  }
  if (is_serial()) {
    cq_status lifecycle = serial_finalise_device(verbosity);
    if (lifecycle != CQ_SUCCESS) status = lifecycle;
  } else if (mpi_env.rank == 0) {
    host_send_ctrl_op(CQ_CTRL_FINALISE, NULL);
    int remote_status;
    MPI_Recv(&remote_status, 1, MPI_INT, CQ_MPI_DEVICE_RANK,
             CQ_MPI_WORLD_COMMS_TAG, CQ_MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    if (remote_status != CQ_SUCCESS) status = remote_status;
  } else {
    device_finalise_comms(verbosity);
    status = dev_comm.status;
  }
  if (!agree_ready(status == CQ_SUCCESS)) status = CQ_ERROR;
  cq_status mpi_status = cleanup_mpi();
  return status == CQ_SUCCESS ? mpi_status : status;
}

cq_status agree_kernel_registration(const char *name, cq_status status, int parameterized) {
  if (!mpi_env.is_init) return status;
  char reference[__CQ_MAX_QKERN_NAME_LENGTH__] = {0};
  if (mpi_env.rank == 0 && name) snprintf(reference, sizeof(reference), "%s", name);
  int kind = parameterized;
  MPI_Bcast(reference, sizeof(reference), MPI_CHAR, 0, registration_comm);
  MPI_Bcast(&kind, 1, MPI_INT, 0, registration_comm);
  int ready = status != CQ_ERROR && name && !strcmp(reference, name) && kind == parameterized;
  if (!agree_ready(ready)) return CQ_ERROR;
  int all_warning = status == CQ_WARNING, result;
  MPI_Allreduce(&all_warning, &result, 1, MPI_INT, MPI_MIN, registration_comm);
  return result ? CQ_WARNING : CQ_SUCCESS;
}

cq_status agree_kernel_shot(cq_exec *exec, cq_status status) {
  pthread_mutex_lock(&exec->lock);
  int halt = exec->halt;
  pthread_mutex_unlock(&exec->lock);
  /* Error has precedence, then early stop. Identical continuation on every
     worker prevents a halted rank stranding peers in a QuEST collective. */
  int classification = status != CQ_SUCCESS && status != CQ_EARLY_SUCCESS ? 2 :
                       (halt || status == CQ_EARLY_SUCCESS ? 1 : 0);
  int agreed = classification;
  if (worker_control_comm != MPI_COMM_NULL)
    MPI_Allreduce(&classification, &agreed, 1, MPI_INT, MPI_MAX, worker_control_comm);
  if (agreed == 2) {
    int error = classification == 2 ? (int)status : INT_MAX, result = error;
    if (worker_control_comm != MPI_COMM_NULL)
      MPI_Allreduce(&error, &result, 1, MPI_INT, MPI_MIN, worker_control_comm);
    return result;
  }
  return agreed == 1 ? CQ_EARLY_SUCCESS : CQ_SUCCESS;
}

void *device_listen(void *unused) {
  pthread_mutex_lock(&dev_comm.comm_lock);
  while (!dev_comm.release) pthread_cond_wait(&dev_comm.cond_comm_busy, &dev_comm.comm_lock);
  bool cancel = dev_comm.cancel;
  pthread_mutex_unlock(&dev_comm.comm_lock);
  if (cancel) return NULL;
  for (;;) {
    int code;
    if (mpi_env.subcomm_rank == 0)
      MPI_Recv(&code, 1, MPI_INT, 0, CQ_MPI_WORLD_COMMS_TAG, CQ_MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    MPI_Bcast(&code, 1, MPI_INT, 0, CQ_MPI_SPLIT_COMM);
    device_dispatch_ctrl_op(code);
    if (code == CQ_CTRL_FINALISE) break;
  }
  return NULL;
}
void device_dispatch_ctrl_op(enum ctrl_code op) {
  switch (op) {
    case CQ_CTRL_FINALISE: {
      device_wait_all_ops();
      for (size_t i=0; i<__CQ_DEVICE_QUEUE_SIZE__; ++i)
        if (executor_handles[i]) device_free_exec(&executor_handles[i]);
      insert_op(op, &mpi_env.verbosity); device_wait_all_ops();
      dev_comm.status = dev_ctrl.lifecycle_status;
      stop_device(); finalise_device_controls(mpi_env.verbosity);
      int local = dev_comm.status, result;
      MPI_Allreduce(&local, &result, 1, MPI_INT, MPI_MIN, CQ_MPI_SPLIT_COMM);
      dev_comm.status = result;
      if (mpi_env.subcomm_rank == 0)
        MPI_Ssend(&result, 1, MPI_INT, 0, CQ_MPI_WORLD_COMMS_TAG, CQ_MPI_COMM_WORLD);
      break;
    }
    case CQ_CTRL_ALLOC: case CQ_CTRL_DEALLOC: {
      device_alloc_params params = {0};
      recv_alloc_params(&params, 0); insert_op(op, &params); device_wait_all_ops();
      send_alloc_params(&params, 0); break;
    }
    case CQ_CTRL_RUN_QKERNEL: case CQ_CTRL_RUN_PQKERNEL: {
      cq_exec *exec = NULL;
      recv_exec_params(&exec, 0);
      if (exec->id >= __CQ_DEVICE_QUEUE_SIZE__ || executor_handles[exec->id])
        MPI_Abort(CQ_MPI_COMM_WORLD, CQ_MPI_RUNTIME_ERROR);
      executor_handles[exec->id] = exec;
      ++num_active_executors;
      insert_op(op, exec); break;
    }
    case CQ_CTRL_WAIT_EXEC: case CQ_CTRL_SYNC_EXEC: case CQ_CTRL_ABORT: {
      size_t id = recv_exec_id(0);
      if (id >= __CQ_DEVICE_QUEUE_SIZE__ || !executor_handles[id])
        MPI_Abort(CQ_MPI_COMM_WORLD, CQ_MPI_RUNTIME_ERROR);
      if (op == CQ_CTRL_ABORT) { comms_exec_halt(executor_handles[id]); break; }
      if (op == CQ_CTRL_WAIT_EXEC) comms_exec_wait(executor_handles[id]);
      send_exec_params(executor_handles[id], 0);
      if (op == CQ_CTRL_WAIT_EXEC) {
        device_free_exec(&executor_handles[id]); --num_active_executors;
      }
      break;
    }
    case CQ_CTRL_WAIT: {
      uint64_t pending = device_wait_all_ops();
      if (mpi_env.subcomm_rank == 0)
        MPI_Ssend(&pending, 1, MPI_UINT64_T, 0, CQ_MPI_WORLD_COMMS_TAG, CQ_MPI_COMM_WORLD);
      break;
    }
    default: break;
  }
}
// ----------------------------------------------------------------------------
// Device Control Paramaters Comms
// ----------------------------------------------------------------------------

void host_comm_params(const enum ctrl_code OP, void * params) {
  const int device_rank = CQ_MPI_DEVICE_RANK;
  switch (OP) {
    case CQ_CTRL_INIT: {
      break;
    }
    case CQ_CTRL_FINALISE: {
      break;
    }
    case CQ_CTRL_ALLOC: {
      send_alloc_params(params, device_rank);
      recv_alloc_params(params, device_rank);
      break;
    }
    case CQ_CTRL_DEALLOC: {
      send_alloc_params(params, device_rank);
      recv_alloc_params(params, device_rank);
      break;
    }
    case CQ_CTRL_RUN_QKERNEL: {
      send_exec_params(params, device_rank);
      break;
    }
    case CQ_CTRL_RUN_PQKERNEL: {
      send_exec_params(params, device_rank);
      break;
    }
    case CQ_CTRL_WAIT_EXEC: {
      cq_exec * exec_params = (cq_exec *)params;
      send_exec_id(((cq_exec *)params)->id, device_rank);
      recv_exec_params(&exec_params, device_rank);
      host_handles[exec_params->id] = NULL;
      break;
    }
    case CQ_CTRL_SYNC_EXEC: {
      cq_exec * exec_params = (cq_exec *)params;
      send_exec_id(((cq_exec *)params)->id, device_rank);
      recv_exec_params(&exec_params, device_rank);
      break;
    }
    case CQ_CTRL_ABORT: {
      send_exec_id(((cq_exec *)params)->id, device_rank);
      break;
    }
    default: {
      break;
    }
  }
}

void recv_alloc_params(device_alloc_params * params, const int src) {
  cq_log("%s [recv_alloc_params]: receiving...\n", get_comm_source());

  if (!cq_validate_mpi_rank(src)) {
    cq_log("%s [recv_exec_params]: encountered input error\n",
           get_comm_source());
    return;
  }

  const size_t params_size = sizeof(device_alloc_params);
  MPI_Status status;

  // device master (rank 0) gets from host from world comm
  if (mpi_env.subcomm_rank == CQ_MPI_DEVICE_MASTER_RANK
      || mpi_env.rank == CQ_MPI_HOST_RANK) {

    MPI_Recv(params, params_size, MPI_BYTE, src, CQ_MPI_WORLD_COMMS_TAG,
             CQ_MPI_COMM_WORLD, &status);

  }
  // device-rank 0 brodcast params to rest of q-workers
  if (mpi_env.rank != CQ_MPI_HOST_RANK) {
    MPI_Bcast(params, params_size, MPI_BYTE, CQ_MPI_DEVICE_MASTER_RANK,
              CQ_MPI_SPLIT_COMM);
  }

  cq_log("%s [recv_alloc_params]: received.\n", get_comm_source());
  print_alloc_params(params);
}

void send_alloc_params(const device_alloc_params * params, const int dest) {
  if (params == NULL) {
    cq_log("%s [recv_exec_params]: passed null params\n", get_comm_source());
    return;
  }

  if (!cq_validate_mpi_rank(dest)) {
    cq_log("%s [recv_exec_params]: encountered input error\n",
           get_comm_source());
    return;
  }

  // device master (rank 0) gets from host from world comm
  if (mpi_env.subcomm_rank == CQ_MPI_DEVICE_MASTER_RANK
      || mpi_env.rank == CQ_MPI_HOST_RANK) {

    cq_log("%s [send_alloc_params]: sending...\n", get_comm_source());
    const size_t params_size = sizeof(device_alloc_params);
    print_alloc_params(params);
    MPI_Ssend(params, params_size, MPI_BYTE, dest, CQ_MPI_WORLD_COMMS_TAG,
              CQ_MPI_COMM_WORLD);
    cq_log("%s [send_alloc_params]: sent.\n", get_comm_source());

  }
}

size_t recv_exec_id(const int src) {
  cq_log("%s [recv_exec_id]: receiving...\n", get_comm_source());
  if (!cq_validate_mpi_rank(src)) {
    cq_log("%s [recv_exec_params]: encountered input error\n",
           get_comm_source());
    return -1;
  }

  size_t id = -1;
  MPI_Status status;

  // device master (rank 0) gets from host from world comm
  if (mpi_env.subcomm_rank == CQ_MPI_DEVICE_MASTER_RANK
      || mpi_env.rank == CQ_MPI_HOST_RANK) {

    MPI_Recv(&id, 1, MPI_UINT64_T, src, CQ_MPI_WORLD_COMMS_TAG,
             CQ_MPI_COMM_WORLD, &status);

  }
  // device-rank 0 brodcast params to rest of q-workers
  if (mpi_env.rank != CQ_MPI_HOST_RANK) {
    MPI_Bcast(&id, 1, MPI_UINT64_T, CQ_MPI_DEVICE_MASTER_RANK,
              CQ_MPI_SPLIT_COMM);
  }

  cq_log("%s [recv_exec_id]: received id: %zu\n", get_comm_source(), id);
  return id;
}

void send_exec_id(const size_t id, const int dest) {
  if (!cq_validate_mpi_rank(dest)) {
    cq_log("%s [recv_exec_params]: encountered input error\n",
           get_comm_source());
    return;
  }

  // device master (rank 0) gets from host from world comm
  if (mpi_env.subcomm_rank == CQ_MPI_DEVICE_MASTER_RANK
      || mpi_env.rank == CQ_MPI_HOST_RANK) {

    cq_log("%s [send_exec_id]: sending id: %zu...\n", get_comm_source(), id);
    MPI_Ssend(&id, 1, MPI_UINT64_T, dest, CQ_MPI_WORLD_COMMS_TAG,
              CQ_MPI_COMM_WORLD);
    cq_log("%s [send_exec_id]: sent\n", get_comm_source());

  }
}

void recv_exec_params(cq_exec ** ehp, const int src) {
  cq_log("%s [recv_exec_params]: receiving...\n", get_comm_source());

  if (ehp == NULL) {
    cq_log("%s [recv_exec_params]: passed null ehp\n", get_comm_source());
    return;
  }

  if (!cq_validate_mpi_rank(src)) {
    cq_log("%s [recv_exec_params]: encountered input error\n",
           get_comm_source());
    return;
  }

  int msg_size;
  MPI_Status status;

  // device-master (rank 0) gets from host from world comm
  // or host gets from device-master
  if (mpi_env.subcomm_rank == CQ_MPI_DEVICE_MASTER_RANK
      || mpi_env.rank == CQ_MPI_HOST_RANK) {

    MPI_Recv(&msg_size, 1, MPI_INT, src, CQ_MPI_WORLD_COMMS_TAG,
             CQ_MPI_COMM_WORLD, &status);

  }
  // device-rank 0 brodcast params to rest of q-workers
  if (mpi_env.rank != CQ_MPI_HOST_RANK) {
    MPI_Barrier(CQ_MPI_SPLIT_COMM);
    MPI_Bcast(&msg_size, 1, MPI_INT, CQ_MPI_DEVICE_MASTER_RANK,
              CQ_MPI_SPLIT_COMM);
  }

  void * recv_buffer = malloc(msg_size);

  // reserve space for all the data + currently unused members
  if (*ehp == NULL) {
    cq_log("%s [recv_exec_params]: *ehp is NULL. Allocating on device.\n",
           get_comm_source());

    *ehp = (cq_exec *)calloc(1, sizeof(cq_exec));
    if (!*ehp) MPI_Abort(CQ_MPI_COMM_WORLD, CQ_MPI_MALLOC_ERROR);
    pthread_mutex_init(&(*ehp)->lock, NULL);
    pthread_cond_init(&(*ehp)->cond_exec_complete, NULL);
  } else {
    cq_log(
        "%s [recv_exec_params]: *ehp is already allocated. So I'm on "
        "host.\n",
        get_comm_source());
  }
  pthread_mutex_lock(&(*ehp)->lock);

  if (recv_buffer == NULL || *ehp == NULL) {
    cq_log("%s [recv_exec_params]: malloc failed. Exiting\n",
           get_comm_source());
    MPI_Abort(CQ_MPI_COMM_WORLD, CQ_MPI_MALLOC_ERROR);
  }

  // device-master (rank 0) gets from host from world comm
  // or host gets from device-master
  if (mpi_env.subcomm_rank == CQ_MPI_DEVICE_MASTER_RANK
      || mpi_env.rank == CQ_MPI_HOST_RANK) {

    MPI_Recv(recv_buffer, msg_size, MPI_PACKED, src, CQ_MPI_WORLD_COMMS_TAG,
             CQ_MPI_COMM_WORLD, &status);

  }
  // device-rank 0 brodcast params to rest of q-workers
  if (mpi_env.rank != CQ_MPI_HOST_RANK) {
    MPI_Barrier(CQ_MPI_SPLIT_COMM);
    MPI_Bcast(recv_buffer, msg_size, MPI_PACKED, CQ_MPI_DEVICE_MASTER_RANK,
              CQ_MPI_SPLIT_COMM);
  }

  const size_t bool_size = sizeof(bool);
  const size_t cq_status_size = sizeof(cq_status);
  // unpack
  int position = 0;
  MPI_Unpack(recv_buffer, msg_size, &position, &(*ehp)->id, 1, MPI_UINT64_T,
             CQ_MPI_COMM_WORLD);
  MPI_Unpack(recv_buffer, msg_size, &position, &(*ehp)->exec_init, bool_size,
             MPI_BYTE, CQ_MPI_COMM_WORLD);
  MPI_Unpack(recv_buffer, msg_size, &position, &(*ehp)->complete, bool_size,
             MPI_BYTE, CQ_MPI_COMM_WORLD);
  MPI_Unpack(recv_buffer, msg_size, &position, &(*ehp)->halt, bool_size,
             MPI_BYTE, CQ_MPI_COMM_WORLD);
  MPI_Unpack(recv_buffer, msg_size, &position, &(*ehp)->status, cq_status_size,
             MPI_BYTE, CQ_MPI_COMM_WORLD);
  MPI_Unpack(recv_buffer, msg_size, &position, &(*ehp)->nqubits, 1,
             MPI_UINT64_T, CQ_MPI_COMM_WORLD);
  MPI_Unpack(recv_buffer, msg_size, &position, &(*ehp)->completed_shots, 1,
             MPI_UINT64_T, CQ_MPI_COMM_WORLD);
  MPI_Unpack(recv_buffer, msg_size, &position, &(*ehp)->expected_shots, 1,
             MPI_UINT64_T, CQ_MPI_COMM_WORLD);
  MPI_Unpack(recv_buffer, msg_size, &position, &(*ehp)->nmeasure, 1,
             MPI_UINT64_T, CQ_MPI_COMM_WORLD);
  MPI_Unpack(recv_buffer, msg_size, &position, &(*ehp)->params_size, 1,
             MPI_UINT64_T, CQ_MPI_COMM_WORLD);

  size_t fname_size = 0;
  MPI_Unpack(recv_buffer, msg_size, &position, &fname_size, 1, MPI_UINT64_T,
             CQ_MPI_COMM_WORLD);
  fname_size *= sizeof(char);

  const size_t qreg_size = sizeof(qubit) * (*ehp)->nqubits;
  const size_t num_shots = (*ehp)->expected_shots;
  const size_t creg_size = sizeof(cstate) * (*ehp)->nmeasure * num_shots;
  const size_t params_size = (*ehp)->params_size;

  // when on host the resources are already allocated!
  if (mpi_env.rank != CQ_MPI_HOST_RANK) {
    (*ehp)->fname = (char *)malloc(fname_size);
    if ((*ehp)->fname == NULL) {
      cq_log("%s [recv_exec_params]: malloc *ehp->fname failed. Exiting\n",
             get_comm_source());
      MPI_Abort(CQ_MPI_COMM_WORLD, CQ_MPI_MALLOC_ERROR);
    }

    (*ehp)->qreg = (qubit *)malloc(qreg_size);
    if ((*ehp)->qreg == NULL) {
      cq_log("%s [recv_exec_params]: malloc *ehp->qreg failed. Exiting\n",
             get_comm_source());
      MPI_Abort(CQ_MPI_COMM_WORLD, CQ_MPI_MALLOC_ERROR);
    }

    (*ehp)->creg = (cstate *)malloc(creg_size ? creg_size : 1);
    if ((*ehp)->creg == NULL) {
      cq_log("%s [recv_exec_params]: malloc *ehp->creg failed. Exiting\n",
             get_comm_source());
      MPI_Abort(CQ_MPI_COMM_WORLD, CQ_MPI_MALLOC_ERROR);
    }

    (*ehp)->params = malloc(params_size ? params_size : 1);
    if ((*ehp)->params == NULL) {
      cq_log("%s [recv_exec_params]: malloc *ehp->params failed. Exiting\n",
             get_comm_source());
      MPI_Abort(CQ_MPI_COMM_WORLD, CQ_MPI_MALLOC_ERROR);
    }
  }

  MPI_Unpack(recv_buffer, msg_size, &position, (*ehp)->fname, fname_size,
             MPI_CHAR, CQ_MPI_COMM_WORLD);
  MPI_Unpack(recv_buffer, msg_size, &position, (char *)(*ehp)->qreg, qreg_size,
             MPI_BYTE, CQ_MPI_COMM_WORLD);
  MPI_Unpack(recv_buffer, msg_size, &position, (char *)(*ehp)->creg, creg_size,
             MPI_BYTE, CQ_MPI_COMM_WORLD);
  MPI_Unpack(recv_buffer, msg_size, &position, (char *)(*ehp)->params,
             params_size, MPI_BYTE, CQ_MPI_COMM_WORLD);

  free(recv_buffer);

  cq_log("%s [recv_exec_params]: received.\n", get_comm_source());
  print_ehp(*ehp);
  pthread_mutex_unlock(&(*ehp)->lock);
}

void send_exec_params(cq_exec * ehp, const int dest) {
  if (ehp == NULL) {
    cq_log("%s [recv_exec_params]: passed null ehp\n", get_comm_source());
    return;
  }

  if (!cq_validate_mpi_rank(dest)) {
    cq_log("%s [recv_exec_params]: encountered input error\n",
           get_comm_source());
    return;
  }

// if running with MPI QuEST, we don't need to communicate
// with quantum workers because the results from QuEST
// (e.g. measurements) should already be synchronised.
// so only device-master communicates its state to host.
  // device master (rank 0) gets from host from world comm
  if (mpi_env.subcomm_rank == CQ_MPI_DEVICE_MASTER_RANK
      || mpi_env.rank == CQ_MPI_HOST_RANK) {

    pthread_mutex_lock(&ehp->lock);
    cq_log("%s [send_exec_params]: sending...\n", get_comm_source());
    print_ehp(ehp);

    const size_t bool_size = sizeof(bool);
    const size_t cq_status_size = sizeof(cq_status);
    int max_buffer_size = 0;
    int member_size = 0;
    MPI_Pack_size(1, MPI_UINT64_T, CQ_MPI_COMM_WORLD,
                  &max_buffer_size);  // id
    MPI_Pack_size(bool_size, MPI_BYTE, CQ_MPI_COMM_WORLD,
                  &member_size);  // exec_init
    max_buffer_size += member_size;
    MPI_Pack_size(bool_size, MPI_BYTE, CQ_MPI_COMM_WORLD,
                  &member_size);  // complete
    max_buffer_size += member_size;
    MPI_Pack_size(bool_size, MPI_BYTE, CQ_MPI_COMM_WORLD,
                  &member_size);  // halt
    max_buffer_size += member_size;
    MPI_Pack_size(cq_status_size, MPI_BYTE, CQ_MPI_COMM_WORLD,
                  &member_size);  // status
    max_buffer_size += member_size;
    MPI_Pack_size(1, MPI_UINT64_T, CQ_MPI_COMM_WORLD,
                  &member_size);  // nqubits
    max_buffer_size += member_size;
    MPI_Pack_size(1, MPI_UINT64_T, CQ_MPI_COMM_WORLD,
                  &member_size);  // completed_shots
    max_buffer_size += member_size;
    MPI_Pack_size(1, MPI_UINT64_T, CQ_MPI_COMM_WORLD,
                  &member_size);  // expected_shots
    max_buffer_size += member_size;
    MPI_Pack_size(1, MPI_UINT64_T, CQ_MPI_COMM_WORLD,
                  &member_size);  // nmeasure
    max_buffer_size += member_size;
    MPI_Pack_size(1, MPI_UINT64_T, CQ_MPI_COMM_WORLD, &member_size);
    max_buffer_size += member_size;  // params_size

    size_t fname_size = 0;
    if (ehp->fname != NULL) {
      fname_size = strlen(ehp->fname) + 1;
    }
    MPI_Pack_size(1, MPI_UINT64_T, CQ_MPI_COMM_WORLD,
                  &member_size);  // fname_size
    max_buffer_size += member_size;
    MPI_Pack_size(fname_size, MPI_CHAR, CQ_MPI_COMM_WORLD,
                  &member_size);  // fname
    max_buffer_size += member_size;

    const size_t qreg_size = sizeof(qubit) * ehp->nqubits;
    MPI_Pack_size(qreg_size, MPI_BYTE, CQ_MPI_COMM_WORLD,
                  &member_size);  // qreg
    max_buffer_size += member_size;

    const size_t num_shots = ehp->expected_shots;
    const size_t creg_size = sizeof(cstate) * ehp->nmeasure * num_shots;

    MPI_Pack_size(creg_size, MPI_BYTE, CQ_MPI_COMM_WORLD,
                  &member_size);  // creg
    max_buffer_size += member_size;

    const size_t params_size = ehp->params_size;
    MPI_Pack_size(params_size, MPI_BYTE, CQ_MPI_COMM_WORLD, &member_size);
    max_buffer_size += member_size;  // params

    void * send_buffer = malloc(max_buffer_size);
    if (send_buffer == NULL) {
      cq_log(
          "Failed to allocate buffer for sending executor handle. Exiting\n");
      MPI_Abort(CQ_MPI_COMM_WORLD, CQ_MPI_MALLOC_ERROR);
    }
    int position = 0;
    MPI_Pack(&ehp->id, 1, MPI_UINT64_T, send_buffer, max_buffer_size, &position,
             CQ_MPI_COMM_WORLD);
    MPI_Pack(&ehp->exec_init, bool_size, MPI_BYTE, send_buffer, max_buffer_size,
             &position, CQ_MPI_COMM_WORLD);
    MPI_Pack(&ehp->complete, bool_size, MPI_BYTE, send_buffer, max_buffer_size,
             &position, CQ_MPI_COMM_WORLD);
    MPI_Pack(&ehp->halt, bool_size, MPI_BYTE, send_buffer, max_buffer_size,
             &position, CQ_MPI_COMM_WORLD);
    MPI_Pack(&ehp->status, cq_status_size, MPI_BYTE, send_buffer,
             max_buffer_size, &position, CQ_MPI_COMM_WORLD);

    MPI_Pack(&ehp->nqubits, 1, MPI_UINT64_T, send_buffer, max_buffer_size,
             &position, CQ_MPI_COMM_WORLD);
    MPI_Pack(&ehp->completed_shots, 1, MPI_UINT64_T, send_buffer,
             max_buffer_size, &position, CQ_MPI_COMM_WORLD);
    MPI_Pack(&ehp->expected_shots, 1, MPI_UINT64_T, send_buffer,
             max_buffer_size, &position, CQ_MPI_COMM_WORLD);
    MPI_Pack(&ehp->nmeasure, 1, MPI_UINT64_T, send_buffer, max_buffer_size,
             &position, CQ_MPI_COMM_WORLD);
    MPI_Pack(&ehp->params_size, 1, MPI_UINT64_T, send_buffer, max_buffer_size,
             &position, CQ_MPI_COMM_WORLD);

    // NOTE: skip pthread stuff...

    MPI_Pack(&fname_size, 1, MPI_UINT64_T, send_buffer, max_buffer_size,
             &position, CQ_MPI_COMM_WORLD);
    MPI_Pack(ehp->fname, fname_size, MPI_CHAR, send_buffer, max_buffer_size,
             &position, CQ_MPI_COMM_WORLD);

    MPI_Pack(ehp->qreg, qreg_size, MPI_BYTE, send_buffer, max_buffer_size,
             &position, CQ_MPI_COMM_WORLD);
    MPI_Pack(ehp->creg, creg_size, MPI_BYTE, send_buffer, max_buffer_size,
             &position, CQ_MPI_COMM_WORLD);
    MPI_Pack(ehp->params, params_size, MPI_BYTE, send_buffer, max_buffer_size,
             &position, CQ_MPI_COMM_WORLD);

    MPI_Ssend(&position, 1, MPI_INT, dest, CQ_MPI_WORLD_COMMS_TAG,
              CQ_MPI_COMM_WORLD);

    MPI_Ssend(send_buffer, position, MPI_PACKED, dest, CQ_MPI_WORLD_COMMS_TAG,
              CQ_MPI_COMM_WORLD);

    free(send_buffer);
    cq_log("%s [send_exec_params]: sent.\n", get_comm_source());
    pthread_mutex_unlock(&ehp->lock);
  }
}

void device_free_exec(cq_exec ** ehp) {
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

const char * get_comm_source(void) {
  if (mpi_env.rank == CQ_MPI_HOST_RANK) {
    return "Host:\t\t\t\t";
  } else if (mpi_env.rank == CQ_MPI_DEVICE_RANK) {
    return "Device:\t\t\t\t";
  } else if (is_quantum_worker()) {
    return debug_worker_name;
  } else {
    return "";
  }
}

const char * op_to_str(const enum ctrl_code OP) {
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

void print_alloc_params(const device_alloc_params * params) {
  if (params == NULL) {
    return;
  }
  cq_log("%s alloc params: NQUBITS: %zu, qreg_idx: %zu, STATUS: %d\n",
         get_comm_source(), params->NQUBITS, params->qregistry_idx,
         params->status);
}

void print_ehp(const cq_exec * ehp) {
  if (ehp == NULL) {
    cq_log("ehp is NULL\n");
  }

  cq_log(
      "%s ehp details:\nid: %zu, exec_init: %d, complete: %d, halt: %d, "
      "STATUS: "
      "%d\nNQUBITS: "
      "%zu, completed_shots: %zu, expected_shots: %zu, NMEASURE: "
      "%zu, params_size: %zu\nfname: "
      "%s\nqreg:\n",
      get_comm_source(), ehp->id, ehp->exec_init, ehp->complete, ehp->halt,
      ehp->status, ehp->nqubits, ehp->completed_shots, ehp->expected_shots,
      ehp->nmeasure, ehp->params_size, ehp->fname);

  for (size_t i = 0; i < ehp->nqubits; ++i) {
    cq_log("qubit[%zu]: reg_idx: %zu, offset: %zu, N: %zu\n", i,
           ehp->qreg[i].registry_index, ehp->qreg[i].offset, ehp->qreg[i].N);
  }
  cq_log("\ncreg:\n");

  //  for (size_t i = 0; i < ehp->nmeasure * ehp->expected_shots; ++i) {
  //    cq_log("cstate[%zu]: %d\n", i, ehp->creg[i]);
  //  }

  cq_log("\nparams:\n");
  double * dparams = (double *)ehp->params;
  for (size_t i = 0; i < ehp->params_size / sizeof(double); ++i) {
    cq_log("params[%zu]: %f\n", i, dparams[i]);
  }
  cq_log("%s ehp details END\n\n", get_comm_source());
}

bool is_quantum_worker(void) {
  return mpi_env.rank > 0;
}
