#include <pthread.h>
#include <stdio.h>
#include <stdbool.h>
#include "comms.h"
#include "src/host/opcodes.h"
#include "src/device/opreg.h"

int initialise_device_link() {
  pthread_mutex_init(&dev_ctrl.device_lock, NULL);
  pthread_cond_init(&dev_ctrl.new_op_condition, NULL);
  pthread_cond_init(&dev_ctrl.ctrl_op_complete_condition, NULL);

  dev_ctrl.run_device = true;
  dev_ctrl.new_op = false;

  pthread_create(&dev_ctrl.device_thread, NULL, &device_control_thread, NULL);

  host_send_ctrl_op(INIT);
  host_wait_ctrl_op();
  
  return 0;
}

void host_send_ctrl_op(const enum ctrl_code OP) {
  pthread_mutex_lock(&dev_ctrl.device_lock);
  dev_ctrl.op = OP;
  dev_ctrl.new_op = true;
  dev_ctrl.ctrl_op_complete = false;
  pthread_cond_signal(&dev_ctrl.new_op_condition);
  pthread_mutex_unlock(&dev_ctrl.device_lock);

  return;
}

void host_wait_ctrl_op() {
  pthread_mutex_lock(&dev_ctrl.device_lock);
  while(!dev_ctrl.ctrl_op_complete) {
    pthread_cond_wait(
      &dev_ctrl.ctrl_op_complete_condition, 
      &dev_ctrl.device_lock
    );
  }
  pthread_mutex_unlock(&dev_ctrl.device_lock);
  return;
}

void * device_control_thread(void * par) {
  enum ctrl_code current_op; 
  
  // selfishly ensure that we start running and
  // acquire resources before the host thread can start
  // trying to throw things at us
  pthread_mutex_lock(&dev_ctrl.device_lock);

  // run_device set to FALSE at cq_finalise
  while (dev_ctrl.run_device) {
   
    while(!dev_ctrl.new_op) {
      // wait for a new op to be posted
      pthread_cond_wait(&dev_ctrl.new_op_condition, &dev_ctrl.device_lock);
    }
    
    current_op = dev_ctrl.op;
    dev_ctrl.new_op = false; 
    
    printf("Device op = %u\n", current_op); 
    control_registry[current_op](par);

    dev_ctrl.ctrl_op_complete = true;
    pthread_cond_signal(&dev_ctrl.ctrl_op_complete_condition);    
  }  
 
  pthread_mutex_unlock(&dev_ctrl.device_lock);

  return NULL;
}

int finalise_device_link() {
  pthread_mutex_lock(&dev_ctrl.device_lock);
  dev_ctrl.run_device = false;
  pthread_mutex_unlock(&dev_ctrl.device_lock);

  host_send_ctrl_op(FINALISE);

  pthread_join(dev_ctrl.device_thread, NULL);

  pthread_cond_destroy(&dev_ctrl.new_op_condition);
  pthread_cond_destroy(&dev_ctrl.ctrl_op_complete_condition);
  pthread_mutex_destroy(&dev_ctrl.device_lock);

  return 0;
}