#include <stdio.h>
#include "env.h"
#include "quest/include/environment.h" 

int cq_init(const unsigned int VERBOSITY) {
  int status = 0;  

  if (VERBOSITY > 0) {
    printf("Initialising CQ Simulated Backend library. QuEST environment report to follow.\n\n");
  }

  initQuESTEnv();

  if (VERBOSITY > 0) {
    reportQuESTEnv();
  }

  if (isQuESTEnvInit() == 0) {
    status = -1;
  }
  
  return status;
}

int cq_finalise() {
  finalizeQuESTEnv();
  return isQuESTEnvInit();
}
