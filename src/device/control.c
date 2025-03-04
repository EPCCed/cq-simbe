#include <stdbool.h>
#include "src/host-device/comms.h"
#include "control.h"

#include <stdio.h>

int initialise_quantum_device(void *) {
  return 0;
}

int abort_current_kernel(void *) {
  return 0;
}

int finalise_quantum_device(void *) {
  printf("Finalising quantum device\n"); 
  dev_ctrl.run_device = false;
  return 0;
}