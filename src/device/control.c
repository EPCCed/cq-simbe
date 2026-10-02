#include <stdio.h>
#include <stdlib.h>
#include "datatypes.h"
#include "src/host-device/comms.h"
#include "control.h"
#include "utils.h"
#include "kernel_utils.h"
#include "resources.h"

#include "quest/include/environment.h"


static bool simulator_owned;

cq_status (*control_registry[8])(void *) =  {
  initialise_simulator,
  abort_current_kernel,
  finalise_simulator,
  device_alloc_qureg,
  device_dealloc_qureg,
  run_qkernel,
  run_pqkernel,
  test_control_fn
};

cq_status initialise_simulator(void *par) {
  if (simulator_owned) return CQ_WARNING;
  cq_status status = init_quest_env();
  if (status != CQ_SUCCESS) return status;
  simulator_owned = true;
  if (*(unsigned int *)par) reportQuESTEnv();
  init_qregistry();
  return CQ_SUCCESS;
}
cq_status abort_current_kernel(void *par) { return CQ_ERROR; }
cq_status finalise_simulator(void *par) {
  if (!simulator_owned) return CQ_WARNING;
  clear_qregistry();
  finalizeQuESTEnv();
  simulator_owned = false;
  return finish_quest_env();
}

cq_status run_qkernel(void * par) {
  cq_exec * p_exec = (cq_exec*) par;

  // find local function pointer
  qkern qk = NULL;
  cq_status status = find_qkern_pointer(p_exec->fname, &qk);

  status = agree_kernel_shot(p_exec, status);
  // All device ranks either execute this kernel or report terminal failure.
  if (status == CQ_SUCCESS) {
    const size_t NSHOTS = p_exec->expected_shots;
    const size_t NQUBITS = p_exec->nqubits;
    const size_t NMEASURE = p_exec->nmeasure;

    cstate *local_creg = malloc((NMEASURE ? NMEASURE : 1) * sizeof(cstate));
    status = agree_kernel_shot(p_exec, local_creg ? CQ_SUCCESS : CQ_ERROR);
    if (status != CQ_SUCCESS) {
      free(local_creg);
      complete_failed_exec(p_exec, status == CQ_EARLY_SUCCESS ? CQ_SUCCESS : status);
      return status;
    }

    for (size_t shot = 0; shot < NSHOTS; ++shot) {
      init_creg(NMEASURE, -1, local_creg);

      status = qk(NQUBITS, p_exec->qreg, NMEASURE, local_creg, NULL);

      status = agree_kernel_shot(p_exec, status);
      device_sync_exec(status, shot, local_creg, p_exec);
      if (status != CQ_SUCCESS) break;
    }

    free(local_creg);
  } else {
    complete_failed_exec(p_exec, status == CQ_EARLY_SUCCESS ? CQ_SUCCESS : status);
  }

  return status;
}

cq_status run_pqkernel(void * par) {
  cq_exec * p_exec = (cq_exec*) par;

  // find local function pointer
  pqkern pqk = NULL;
  cq_status status = find_pqkern_pointer(p_exec->fname, &pqk);

  status = agree_kernel_shot(p_exec, status);
  // All device ranks either execute this kernel or report terminal failure.
  if (status == CQ_SUCCESS) {
    const size_t NSHOTS = p_exec->expected_shots;
    const size_t NQUBITS = p_exec->nqubits;
    const size_t NMEASURE = p_exec->nmeasure;

    cstate *local_creg = malloc((NMEASURE ? NMEASURE : 1) * sizeof(cstate));
    status = agree_kernel_shot(p_exec, local_creg ? CQ_SUCCESS : CQ_ERROR);
    if (status != CQ_SUCCESS) {
      free(local_creg);
      complete_failed_exec(p_exec, status == CQ_EARLY_SUCCESS ? CQ_SUCCESS : status);
      return status;
    }

    for (size_t shot = 0; shot < NSHOTS; ++shot) {
      init_creg(NMEASURE, -1, local_creg);

      status = pqk(NQUBITS, p_exec->qreg, NMEASURE, local_creg, p_exec->params, NULL);

      status = agree_kernel_shot(p_exec, status);
      device_sync_exec(status, shot, local_creg, p_exec);
      if (status != CQ_SUCCESS) break;
    }

    free(local_creg);
  } else {
    complete_failed_exec(p_exec, status == CQ_EARLY_SUCCESS ? CQ_SUCCESS : status);
  }

 return status;
}

cq_status test_control_fn(void * par) {
  unsigned int * p_test_count = (unsigned int *) par;

  (*p_test_count)++;

  return CQ_SUCCESS;
}
