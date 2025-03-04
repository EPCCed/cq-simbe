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

  dev_ctrl.run_device = true;
  pthread_mutex_init(&dev_ctrl.device_lock, NULL);
  pthread_cond_init(&dev_ctrl.new_op_condition, NULL);
  pthread_create(&dev_ctrl.device_thread, NULL, &device_watch_thread, NULL);
  printf("Device thread created\n"); 
  sleep(5); 

  return status;
}

int cq_finalise() {
  printf("Host finalising\n");
  pthread_mutex_lock(&dev_ctrl.device_lock);
  dev_ctrl.op = FINALISE;
  pthread_cond_signal(&dev_ctrl.new_op_condition);
  pthread_mutex_unlock(&dev_ctrl.device_lock);
  printf("Host signaled device\n");

  pthread_mutex_lock(&dev_ctrl.device_lock);
  pthread_join(dev_ctrl.device_thread, NULL);
  pthread_mutex_unlock(&dev_ctrl.device_lock);

  pthread_mutex_destroy(&dev_ctrl.device_lock);
  pthread_cond_destroy(&dev_ctrl.new_op_condition);

  return 0;
}
