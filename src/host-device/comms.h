#ifndef HOST_DEVICE_COMMS_H
#define HOST_DEVICE_COMMS_H

#include <stdbool.h>
#include <pthread.h>
#include "src/host/opcodes.h"

struct dev_link {
  bool run_device;
  pthread_mutex_t device_lock;
  pthread_cond_t new_op_condition;
  pthread_t device_thread;
  enum ctrl_code op;
};

extern struct dev_link dev_ctrl;

void * device_watch_thread();

#endif