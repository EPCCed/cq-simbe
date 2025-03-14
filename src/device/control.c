#include <stdbool.h>
#include <stdio.h>
#include "src/host-device/comms.h"
#include "control.h"
#include "kernels.h"
#include "resources.h"
#include "quest/include/environment.h"

int (*control_registry[7])(void *) =  {
  initialise_simulator,
  abort_current_kernel,
  finalise_simulator,
  device_alloc_qureg,
  device_dealloc_qureg,
  run_qkernel,
  run_pqkernel
};

int initialise_simulator(void * par) {
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

  return 0;
}

int abort_current_kernel(void * par) {
  return 0;
}

int finalise_simulator(void * par) {
  const unsigned int * pVERBOSITY = (const unsigned int *) par;
  
  if (*pVERBOSITY > 0) {
    printf("Finalising QuEST\n");
  }

  clear_qregistry();

  finalizeQuESTEnv();

  return 0;
}

int run_qkernel(void * par) {
  qkern_params * qk_par = (qkern_params*) par;

  // find local function pointer
  qkern qk = NULL;
  int status = find_qkern_pointer(qk_par->FNAME, &qk);

  // run it!
  if (!status) {
    qk(qk_par->NQUBITS, qk_par->qreg, qk_par->creg, NULL);
  }

  return status;
}

int run_pqkernel(void * par) {
  qkern_params * pqk_par = (qkern_params*) par;

  // find local function pointer
  pqkern pqk = NULL;
  int status = find_pqkern_pointer(pqk_par->FNAME, &pqk);

  // run it!
  if (!status) {
    pqk(pqk_par->NQUBITS, pqk_par->qreg, pqk_par->creg, pqk_par->params, NULL);
  }

 return status;
}
