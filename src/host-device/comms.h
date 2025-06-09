#ifndef HOST_DEVICE_COMMS_H
#define HOST_DEVICE_COMMS_H

#include <stdbool.h>
#include <pthread.h>
#include "datatypes.h"
#include "src/host/opcodes.h"

#define __CQ_DEVICE_QUEUE_SIZE__ 16

struct dev_link {
  bool run_device;
  pthread_t device_thread;
  pthread_mutex_t device_lock;

  bool device_busy;
  pthread_cond_t cond_device_busy;
  
  size_t num_ops;
  pthread_cond_t cond_queue_empty;
  pthread_cond_t cond_queue_full;

  size_t next_op_in;
  size_t next_op_out;
  enum ctrl_code op_buffer[__CQ_DEVICE_QUEUE_SIZE__];
  void * op_params_buffer[__CQ_DEVICE_QUEUE_SIZE__];
};

typedef struct device_alloc_params {
  const size_t NQUBITS;
  size_t qregistry_idx;
  cq_status status;
} device_alloc_params;

extern struct dev_link dev_ctrl;

int initialise_device(const unsigned int VERBOSITY);

size_t host_send_ctrl_op(const enum ctrl_code OP, void * ctrl_params);

size_t host_sync_exec(cq_exec * const ehp);

size_t host_wait_exec(cq_exec * const ehp);

size_t host_wait_all_ops();

size_t device_sync_exec(const cq_status STATUS, const size_t SHOT, 
  cstate const * const RESULT, cq_exec * ehp);

void * device_control_thread(void *);

int finalise_device(const unsigned int VERBOSITY);

#endif