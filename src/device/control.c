#include <stdbool.h>
#include "src/host-device/comms.h"
#include "control.h"

#include <stdio.h>

int initialise_quantum_device(void *) {
  printf("Initialising quantum device\n");
  return 0;
}

int abort_current_kernel(void *) {
  return 0;
}

int finalise_quantum_device(void *) {
  printf("Finalising quantum device\n"); 
  return 0;
}