#ifndef HOST_DEVICE_COMMS_H
#define HOST_DEVICE_COMMS_H

#include <stdbool.h>
#include <pthread.h>
#include "src/host/opcodes.h"
#include "src/device/resources.h"

struct dev_link {
  bool run_device;
  pthread_mutex_t device_lock;
  bool new_op;
  pthread_cond_t new_op_condition;
  bool ctrl_op_complete; 
  pthread_cond_t ctrl_op_complete_condition;
  pthread_t device_thread;
  enum ctrl_code op;
  void * op_params;
};

extern struct dev_link dev_ctrl;

int initialise_device(const unsigned int VERBOSITY);

void host_send_ctrl_op(const enum ctrl_code, void * ctrl_params);

void host_wait_ctrl_op();

void * device_control_thread(void *);

int finalise_device(const unsigned int VERBOSITY);

#endif