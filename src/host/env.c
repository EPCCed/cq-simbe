#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include "src/host-device/comms.h"
#include "opcodes.h"
#include "env.h"

#include <unistd.h>

struct dev_link dev_ctrl;

int cq_init(const unsigned int VERBOSITY) {
  int status = 0;  

  if (VERBOSITY > 0) {
    printf("Initialising CQ Simulated Backend library. QuEST environment report to follow.\n\n");
  }

  initialise_device_link();
  
  return status;
}

int cq_finalise() {
  printf("Host finalising\n");

  finalise_device_link();

  return 0;
}
