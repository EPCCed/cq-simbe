#include <stdbool.h>
#include <stdio.h>
#include "src/host-device/comms.h"
#include "control.h"
#include "quest/include/environment.h"

int (*control_registry[3])(void *) =  {
  initialise_simulator,
  abort_current_kernel,
  finalise_simulator
};

int initialise_simulator(void * par) {
  const unsigned int * pVERBOSITY = (const unsigned int *) par;

  if (*pVERBOSITY > 0) {
    printf("Initialising QuEST.\n");
  }

  initQuESTEnv();

  if (*pVERBOSITY > 0) {
    reportQuESTEnv();
  }

  return 0;
}

int abort_current_kernel(void *) {
  return 0;
}

int finalise_simulator(void * par) {
  const unsigned int * pVERBOSITY = (const unsigned int *) par;
  
  if (*pVERBOSITY > 0) {
    printf("Finalising QuEST\n");
  }

  finalizeQuESTEnv();

  return 0;
}