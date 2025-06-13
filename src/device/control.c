#include <stdio.h>
#include <stdlib.h>
#include "datatypes.h"
#include "src/host-device/comms.h"
#include "control.h"
#include "utils.h"
#include "kernel_utils.h"
#include "resources.h"
#include "quest/include/environment.h"

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

cq_status initialise_simulator(void * par) {
  cq_status status = CQ_WARNING;
  
  // isQuESTEnvInit returns 1 for true, 0 for false
  if (!isQuESTEnvInit()) {
    const unsigned int * pVERBOSITY = (const unsigned int *) par;

    if (*pVERBOSITY > 0) {
      printf("Initialising QuEST.\n");
    }

    initQuESTEnv();

    if (*pVERBOSITY > 0) {
      reportQuESTEnv();
    
      printf("Initialising quantum resource registry.\n");
    }

    init_qregistry();
  
    status = isQuESTEnvInit() - 1;
  }

  return status;
}

cq_status abort_current_kernel(void * par) {
  return CQ_ERROR;
}

cq_status finalise_simulator(void * par) {
  cq_status status = CQ_WARNING;

  if (isQuESTEnvInit()) {
    const unsigned int * pVERBOSITY = (const unsigned int *) par;
    
    if (*pVERBOSITY > 0) {
      printf("Finalising QuEST\n");
    }

    clear_qregistry();

    finalizeQuESTEnv();

    // isQuESTEnvInit returns 1 for true, 0 for false
    if (!isQuESTEnvInit()) {
      status = CQ_SUCCESS;
    }
  }

  return status;
}

cq_status run_qkernel(void * par) {
  cq_exec * p_exec = (cq_exec*) par;

  // find local function pointer
  qkern qk = NULL;
  cq_status status = find_qkern_pointer(p_exec->fname, &qk);

  // run it!
  if (status == CQ_SUCCESS) {
    const size_t NSHOTS = p_exec->expected_shots;
    const size_t NQUBITS = p_exec->nqubits;
    const size_t NMEASURE = p_exec->nmeasure;

    cstate * local_creg = (cstate*) malloc(NMEASURE * sizeof(cstate));

    for (size_t shot = 0; shot < NSHOTS; ++shot) {
      init_creg(NMEASURE, -1, local_creg);

      status = qk(NQUBITS, p_exec->qreg, local_creg, NULL);

      device_sync_exec(status, shot, local_creg, p_exec);
      if (status != CQ_SUCCESS || p_exec->halt) break;
    }

    free(local_creg);
  }

  return status;
}

cq_status run_pqkernel(void * par) {
  cq_exec * p_exec = (cq_exec*) par;

  // find local function pointer
  pqkern pqk = NULL;
  cq_status status = find_pqkern_pointer(p_exec->fname, &pqk);

  // run it!
  if (status == CQ_SUCCESS) {
    const size_t NSHOTS = p_exec->expected_shots;
    const size_t NQUBITS = p_exec->nqubits;
    const size_t NMEASURE = p_exec->nmeasure;

    cstate * local_creg = (cstate*) malloc(NMEASURE * sizeof(cstate));

    for (size_t shot = 0; shot < NSHOTS; ++shot) {
      init_creg(NMEASURE, -1, local_creg);

      status = pqk(NQUBITS, p_exec->qreg, local_creg, p_exec->params, NULL);

      device_sync_exec(status, shot, local_creg, p_exec);
      if (status != CQ_SUCCESS || p_exec->halt) break;
    }

    free(local_creg);
  }

 return status;
}

cq_status test_control_fn(void * par) {
  unsigned int * p_test_count = (unsigned int *) par;

  (*p_test_count)++;

  return CQ_SUCCESS;
}