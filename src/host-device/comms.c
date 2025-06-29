#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "datatypes.h"
#include "kernel_utils.h"
#include "comms.h"
#include "src/host/opcodes.h"
#include "src/device/control.h"

int initialise_device(const unsigned int VERBOSITY) {
  if (VERBOSITY > 0) {
    printf("Initialising device.\n");
  }
  pthread_mutex_init(&dev_ctrl.device_lock, NULL);
  pthread_cond_init(&dev_ctrl.cond_device_busy, NULL);
  pthread_cond_init(&dev_ctrl.cond_queue_empty, NULL);
  pthread_cond_init(&dev_ctrl.cond_queue_full, NULL);

  dev_ctrl.run_device = true;
  dev_ctrl.device_busy = true;
  dev_ctrl.num_ops = 0;
  dev_ctrl.next_op_in = 0;
  dev_ctrl.next_op_out = 0;

  for (size_t i = 0; i < __CQ_DEVICE_QUEUE_SIZE__; ++i) {
    dev_ctrl.op_buffer[i] = CQ_CTRL_IDLE;
    dev_ctrl.op_params_buffer[i] = NULL;
  }

  pthread_create(&dev_ctrl.device_thread, NULL, &device_control_thread, NULL);

  unsigned int verbosity = VERBOSITY;
  host_send_ctrl_op(CQ_CTRL_INIT, &verbosity);
  host_wait_all_ops();

  return 0;
}

size_t host_send_ctrl_op(const enum ctrl_code OP, void * ctrl_params) {
  pthread_mutex_lock(&dev_ctrl.device_lock);

  while (dev_ctrl.num_ops >= __CQ_DEVICE_QUEUE_SIZE__) {
    // the control queue is full!
    // we'll wait for it to not be full
    pthread_cond_wait(&dev_ctrl.cond_queue_full, &dev_ctrl.device_lock);
  }

  dev_ctrl.op_buffer[dev_ctrl.next_op_in] = OP;
  dev_ctrl.op_params_buffer[dev_ctrl.next_op_in] = ctrl_params;
  ++dev_ctrl.num_ops;

  // It's a ring buffer! 
  // advance next_op_in then mod out buffer size
  ++dev_ctrl.next_op_in;
  dev_ctrl.next_op_in %= __CQ_DEVICE_QUEUE_SIZE__;

  pthread_cond_signal(&dev_ctrl.cond_queue_empty);
  pthread_mutex_unlock(&dev_ctrl.device_lock);

  return dev_ctrl.num_ops;
}

size_t host_sync_exec(cq_exec * const ehp) {
  size_t completed_shots = 0;
  pthread_mutex_lock(&(ehp->lock));
  completed_shots = ehp->completed_shots;
  pthread_mutex_unlock(&(ehp->lock));
  return completed_shots;
}

size_t host_wait_exec(cq_exec * const ehp) {
  pthread_mutex_lock(&(ehp->lock));
  while (!ehp->complete) {
    pthread_cond_wait(
      &(ehp->cond_exec_complete), 
      &(ehp->lock)
    );
  }
  pthread_mutex_unlock(&(ehp->lock));
  return ehp->completed_shots;
}

size_t host_wait_all_ops() {
  pthread_mutex_lock(&dev_ctrl.device_lock);
  while(dev_ctrl.num_ops > 0 || dev_ctrl.device_busy) {
    pthread_cond_wait(
      &dev_ctrl.cond_device_busy, 
      &dev_ctrl.device_lock
    );
  }
  pthread_mutex_unlock(&dev_ctrl.device_lock);
  return dev_ctrl.num_ops;
}

size_t device_sync_exec(const cq_status STATUS, const size_t SHOT, 
cstate const * const RESULT, cq_exec * ehp) {
  pthread_mutex_lock(&ehp->lock);
  
  if (STATUS == CQ_EARLY_SUCCESS) {
    // generally speaking we should respect the kernel-provided
    // status code, but CQ_EARLY_SUCCESS really means CQ_SUCCESS
    // but needed to be != so we could break execution
    ehp->status = CQ_SUCCESS;
  } else {
    ehp->status = STATUS;
  }
  
  ehp->completed_shots += 1;

  // copy local result register to exec
  cstate * dest_creg = ehp->creg + SHOT * ehp->nmeasure;
  memcpy(dest_creg, RESULT, ehp->nmeasure * sizeof(cstate));
  
  // check if the whole execution is done
  if (ehp->completed_shots == ehp->expected_shots || STATUS != CQ_SUCCESS) {
    ehp->complete = true;
    pthread_cond_signal(&(ehp->cond_exec_complete));
  }

  pthread_mutex_unlock(&ehp->lock);

  return SHOT;
}

void * device_control_thread(void * par) {
  enum ctrl_code current_op = CQ_CTRL_IDLE;
  void * current_op_params = NULL;

  // run_device set to FALSE at cq_finalise
  while (dev_ctrl.run_device) {
    pthread_mutex_lock(&dev_ctrl.device_lock);

    while(dev_ctrl.num_ops <= 0) {
      // wait for a new op to be posted
      dev_ctrl.device_busy = false;
      pthread_cond_signal(&dev_ctrl.cond_device_busy);
      pthread_cond_wait(&dev_ctrl.cond_queue_empty, &dev_ctrl.device_lock);
    }
    
    dev_ctrl.device_busy = true;

    // take the next op and params out of the dev_ctrl buffer, and then tidy up the dev_ctrl buffer
    current_op = dev_ctrl.op_buffer[dev_ctrl.next_op_out];
    current_op_params = dev_ctrl.op_params_buffer[dev_ctrl.next_op_out];
    dev_ctrl.op_buffer[dev_ctrl.next_op_out] = CQ_CTRL_IDLE;
    dev_ctrl.op_params_buffer[dev_ctrl.next_op_out] = NULL;

    // decrease the number of queued operations and advance next_op_out
    --dev_ctrl.num_ops;
    ++dev_ctrl.next_op_out;
    dev_ctrl.next_op_out %= __CQ_DEVICE_QUEUE_SIZE__;

    // signal that the queue is no longer full and then relinquish mutex
    pthread_cond_signal(&dev_ctrl.cond_queue_full);
    pthread_mutex_unlock(&dev_ctrl.device_lock);

    control_registry[current_op](current_op_params);
  }  
 
  pthread_mutex_unlock(&dev_ctrl.device_lock);

  return NULL;
}

int finalise_device(const unsigned int VERBOSITY) {
  // Politely wait for the device to finish its current business
  // otherwise setting dev_ctrl.run_device = false might break
  // some stuff, and this should only be called in cq_finalise()
  host_wait_all_ops();

  if (VERBOSITY > 0) {
    printf("Finalising device.\n");
  }

  pthread_mutex_lock(&dev_ctrl.device_lock);
  dev_ctrl.run_device = false;
  pthread_mutex_unlock(&dev_ctrl.device_lock);

  unsigned int verbosity = VERBOSITY;
  host_send_ctrl_op(CQ_CTRL_FINALISE, &verbosity);

  pthread_join(dev_ctrl.device_thread, NULL);

  dev_ctrl.device_busy = false;

  pthread_cond_destroy(&dev_ctrl.cond_device_busy);
  pthread_cond_destroy(&dev_ctrl.cond_queue_empty);
  pthread_cond_destroy(&dev_ctrl.cond_queue_full);
  pthread_mutex_destroy(&dev_ctrl.device_lock);

  return 0;
}