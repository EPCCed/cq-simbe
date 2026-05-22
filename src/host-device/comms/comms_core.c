#include "comms_core.h"

#include "../comms.h"
#include "src/device/control.h"

#include <stdio.h>
#include <string.h>

static size_t global_exec_id_counter = -1;

int init_device_controls(const unsigned int VERBOSITY) {
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

  return 0;
}

void stop_device(void) {
  pthread_mutex_lock(&dev_ctrl.device_lock);
  dev_ctrl.run_device = false;
  pthread_mutex_unlock(&dev_ctrl.device_lock);
}

int finalise_device_controls(const unsigned int VERBOSITY) {
  pthread_join(dev_ctrl.device_thread, NULL);

  dev_ctrl.device_busy = false;

  pthread_cond_destroy(&dev_ctrl.cond_device_busy);
  pthread_cond_destroy(&dev_ctrl.cond_queue_empty);
  pthread_cond_destroy(&dev_ctrl.cond_queue_full);
  pthread_mutex_destroy(&dev_ctrl.device_lock);

  return 0;
}

// NOTE: aka host_send_ctrl_op from OG comm.c
size_t insert_op(const enum ctrl_code OP, void* ctrl_params) {
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

size_t comms_exec_sync(cq_exec* const ehp) {
  size_t completed_shots = 0;
  pthread_mutex_lock(&(ehp->lock));
  completed_shots = ehp->completed_shots;
  pthread_mutex_unlock(&(ehp->lock));
  return completed_shots;
}

size_t comms_exec_wait(cq_exec* const ehp) {
  pthread_mutex_lock(&(ehp->lock));
  while (!ehp->complete) {
    pthread_cond_wait(&(ehp->cond_exec_complete), &(ehp->lock));
  }
  pthread_mutex_unlock(&(ehp->lock));
  return ehp->completed_shots;
}

void comms_exec_halt(cq_exec* const ehp) {
  pthread_mutex_lock(&(ehp->lock));
  ehp->halt = true;
  pthread_mutex_unlock(&(ehp->lock));
  return;
}

// NOTE: aka host_wait_all_ops from OG comm.c
size_t device_wait_all_ops(void) {
  pthread_mutex_lock(&dev_ctrl.device_lock);
  while (dev_ctrl.num_ops > 0 || dev_ctrl.device_busy) {
    pthread_cond_wait(&dev_ctrl.cond_device_busy, &dev_ctrl.device_lock);
  }

  pthread_mutex_unlock(&dev_ctrl.device_lock);
  return dev_ctrl.num_ops;
}

void* device_control_thread(void* par) {
  enum ctrl_code current_op = CQ_CTRL_IDLE;
  void* current_op_params = NULL;

  // run_device set to FALSE at cq_finalise
  while (dev_ctrl.run_device) {
    pthread_mutex_lock(&dev_ctrl.device_lock);

    while (dev_ctrl.num_ops <= 0) {
      // wait for a new op to be posted
      dev_ctrl.device_busy = false;
      pthread_cond_signal(&dev_ctrl.cond_device_busy);
      pthread_cond_wait(&dev_ctrl.cond_queue_empty, &dev_ctrl.device_lock);
    }

    dev_ctrl.device_busy = true;

    // take the next op and params out of the dev_ctrl buffer, and then tidy up
    // the dev_ctrl buffer
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

size_t device_sync_exec(const cq_status STATUS,
                        const size_t SHOT,
                        cstate const* const RESULT,
                        cq_exec* ehp) {
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
  cstate* dest_creg = ehp->creg + SHOT * ehp->nmeasure;
  memcpy(dest_creg, RESULT, ehp->nmeasure * sizeof(cstate));

  // check if the whole execution is done
  if (ehp->completed_shots == ehp->expected_shots || STATUS != CQ_SUCCESS ||
      ehp->halt) {
    ehp->complete = true;
    pthread_cond_signal(&(ehp->cond_exec_complete));
  }

  pthread_mutex_unlock(&ehp->lock);

  return SHOT;
}

size_t assign_exec_id(void) {
  ++global_exec_id_counter;
  global_exec_id_counter %= __CQ_DEVICE_QUEUE_SIZE__;
  return global_exec_id_counter;
}
