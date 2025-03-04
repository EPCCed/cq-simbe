#include <stdlib.h>
#include <pthread.h>
#include "comms.h"
#include "src/device/opreg.h"

#include <stdio.h>

void * device_watch_thread() {
  printf("Device watch thread starting\n");
  pthread_mutex_lock(&dev_ctrl.device_lock);
  while(dev_ctrl.run_device) {
    printf("Device waiting\n");
    pthread_cond_wait(&dev_ctrl.new_op_condition, &dev_ctrl.device_lock);
    printf("Device received signal\n");
    control_registry[dev_ctrl.op](NULL);
  }
  printf("Device stopped running,\n");
  pthread_mutex_unlock(&dev_ctrl.device_lock);

  printf("Device watch thread exiting\n");
  exit(0);
}