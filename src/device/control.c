#include <stdbool.h>
#include <stdio.h>
#include "src/host-device/comms.h"
#include "control.h"
#include "resources.h"
#include "quest/include/environment.h"

int (*control_registry[5])(void *) =  {
  initialise_simulator,
  abort_current_kernel,
  finalise_simulator,
  device_alloc_qureg,
  device_dealloc_qureg
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