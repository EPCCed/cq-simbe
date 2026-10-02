#include "src/host-device/comms.h"

#include "comms_core.h"

#include "quest/include/environment.h"
#include "quest/include/config.h"
#include "src/device/mpi_runtime.h"
#if QUEST_COMPILE_MPI
#include "quest/include/experimental.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include "kernel_utils.h"

struct pending_exec {
  cq_exec *exec;
  struct pending_exec *next;
};
static struct pending_exec *pending;

bool exec_is_live(const cq_exec *exec) {
  for (struct pending_exec *p=pending; p; p=p->next)
    if (p->exec == exec) return true;
  return false;
}
static void release_exec(cq_exec *exec) {
  struct pending_exec **p = &pending;
  while (*p) {
    if ((*p)->exec == exec) {
      struct pending_exec *old = *p;
      *p = old->next; free(old); return;
    }
    p = &(*p)->next;
  }
}

int initialise_device(const unsigned int VERBOSITY) {
  return serial_initialise_device(VERBOSITY);
}

size_t host_send_ctrl_op(const enum ctrl_code OP, void * ctrl_params) {
  size_t result = serial_host_send_ctrl_op(OP, ctrl_params);
  if (OP == CQ_CTRL_WAIT_EXEC) release_exec(ctrl_params);
  return result;
}

size_t host_wait_all_ops(void) {
  return serial_host_wait_all_ops();
}

void host_device_sync_comms(void) {
  serial_host_device_sync_comms();
}

int finalise_device(const unsigned int VERBOSITY) {
  cq_status status = CQ_SUCCESS;
  while (pending) {
    cq_exec *exec = pending->exec;
    host_send_ctrl_op(CQ_CTRL_WAIT_EXEC, exec);
    if (exec->status != CQ_SUCCESS) status = CQ_ERROR;
    finalise_exec_handle(exec);
  }
  cq_status lifecycle = serial_finalise_device(VERBOSITY);
  return lifecycle == CQ_SUCCESS ? status : lifecycle;
}

bool is_device(void) {
  return false;
}

cq_status init_quest_env(void) {
  if (cq_mpi_prepare(!isQuESTEnvInit()) != CQ_SUCCESS) return CQ_ERROR;
#if QUEST_COMPILE_MPI
  initCustomMpiCommQuESTEnv(MPI_COMM_WORLD, -1, -1);
#else
  initQuESTEnv();
#endif
  return isQuESTEnvInit() ? CQ_SUCCESS : CQ_ERROR;
}
cq_status finish_quest_env(void) { return cq_mpi_finish(); }
cq_status agree_kernel_registration(const char *name, cq_status status, int parameterized) {
  return status;
}
cq_status agree_kernel_shot(cq_exec *exec, cq_status status) {
  pthread_mutex_lock(&exec->lock);
  bool halt = exec->halt;
  pthread_mutex_unlock(&exec->lock);
  return cq_mpi_agree_status(status, halt);
}
cq_status submit_exec(enum ctrl_code op, cq_exec *exec) {
  if (!dev_ctrl.worker_started || exec_is_live(exec)) return CQ_ERROR;
  struct pending_exec *node = malloc(sizeof(*node));
  if (!node) return CQ_ERROR;
  node->exec = exec; node->next = pending; pending = node;
  host_send_ctrl_op(op, exec);
  return CQ_SUCCESS;
}
