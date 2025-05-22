#include <stdio.h>
#include "datatypes.h"
#include "src/host-device/comms.h"
#include "control.h"
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
  qkern_params * qk_par = (qkern_params*) par;

  // find local function pointer
  qkern qk = NULL;
  int status = find_qkern_pointer(qk_par->fname, &qk);

  // run it!
  if (!status) {
    status = qk(qk_par->nqubits, qk_par->qreg, qk_par->creg, NULL);
  }

  return status;
}

cq_status run_pqkernel(void * par) {
  qkern_params * pqk_par = (qkern_params*) par;

  // find local function pointer
  pqkern pqk = NULL;
  int status = find_pqkern_pointer(pqk_par->fname, &pqk);

  // run it!
  if (!status) {
    status = pqk(pqk_par->nqubits, pqk_par->qreg, pqk_par->creg, pqk_par->params, NULL);
  }

 return status;
}

cq_status test_control_fn(void * par) {
  unsigned int * p_test_count = (unsigned int *) par;

  (*p_test_count)++;

  return CQ_SUCCESS;
}