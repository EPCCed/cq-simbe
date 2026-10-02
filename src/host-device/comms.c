#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "datatypes.h"
#include "kernel_utils.h"
#include "comms.h"
#include "src/host/opcodes.h"
#include "src/device/control.h"
#include "src/device/mpi_runtime.h"
#include "quest/include/environment.h"

static void destroy_device_sync(void) {
  pthread_cond_destroy(&dev_ctrl.cond_queue_full);
  pthread_cond_destroy(&dev_ctrl.cond_queue_empty);
  pthread_cond_destroy(&dev_ctrl.cond_device_busy);
  pthread_mutex_destroy(&dev_ctrl.device_lock);
}

static void stop_device(void) {
  pthread_mutex_lock(&dev_ctrl.device_lock);
  dev_ctrl.run_device = false;
  pthread_cond_broadcast(&dev_ctrl.cond_queue_empty);
  pthread_mutex_unlock(&dev_ctrl.device_lock);
  pthread_join(dev_ctrl.device_thread, NULL);
  dev_ctrl.worker_started = false;
  destroy_device_sync();
}

int initialise_device(const unsigned int VERBOSITY) {
  if (dev_ctrl.worker_started)
    return CQ_WARNING;
  if (cq_mpi_preflight(!isQuESTEnvInit()) != CQ_SUCCESS)
    return CQ_ERROR;
  if (VERBOSITY > 0)
    printf("Initialising device.\n");

  /* Each error path destroys only primitives successfully initialised. */
  if (pthread_mutex_init(&dev_ctrl.device_lock, NULL) != 0)
    goto fail_preflight;
  if (pthread_cond_init(&dev_ctrl.cond_device_busy, NULL) != 0)
    goto fail_mutex;
  if (pthread_cond_init(&dev_ctrl.cond_queue_empty, NULL) != 0)
    goto fail_busy;
  if (pthread_cond_init(&dev_ctrl.cond_queue_full, NULL) != 0)
    goto fail_empty;

  dev_ctrl.run_device = true;
  dev_ctrl.device_busy = true;
  dev_ctrl.lifecycle_status = CQ_ERROR;
  dev_ctrl.num_ops = 0;
  dev_ctrl.next_op_in = 0;
  dev_ctrl.next_op_out = 0;
  for (size_t i = 0; i < __CQ_DEVICE_QUEUE_SIZE__; ++i) {
    dev_ctrl.op_buffer[i] = CQ_CTRL_IDLE;
    dev_ctrl.op_params_buffer[i] = NULL;
  }
  if (pthread_create(&dev_ctrl.device_thread, NULL, &device_control_thread, NULL) != 0) {
    dev_ctrl.run_device = false;
    dev_ctrl.device_busy = false;
    pthread_cond_destroy(&dev_ctrl.cond_queue_full);
    goto fail_empty;
  }
  dev_ctrl.worker_started = true;
  unsigned int verbosity = VERBOSITY;
  host_send_ctrl_op(CQ_CTRL_INIT, &verbosity);
  host_wait_all_ops();
  cq_status status = dev_ctrl.lifecycle_status;
  if (status != CQ_SUCCESS) {
    stop_device();
    cq_mpi_cancel_preflight();
  }
  return status;

fail_empty:
  pthread_cond_destroy(&dev_ctrl.cond_queue_empty);
fail_busy:
  pthread_cond_destroy(&dev_ctrl.cond_device_busy);
fail_mutex:
  pthread_mutex_destroy(&dev_ctrl.device_lock);
fail_preflight:
  /* Peers may already be preparing MPI on their workers. Join their readiness
     agreement even when this rank could not create a worker. If MPI is owned,
     this failure-only path initialises and finalises it on this caller. */
  cq_mpi_prepare(0);
  cq_mpi_cancel_preflight();
  return CQ_ERROR;
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
  pthread_mutex_lock(&dev_ctrl.device_lock);
  for (;;) {
    while (dev_ctrl.num_ops == 0 && dev_ctrl.run_device) {
      dev_ctrl.device_busy = false;
      pthread_cond_broadcast(&dev_ctrl.cond_device_busy);
      pthread_cond_wait(&dev_ctrl.cond_queue_empty, &dev_ctrl.device_lock);
    }
    if (!dev_ctrl.run_device && dev_ctrl.num_ops == 0)
      break;

    dev_ctrl.device_busy = true;
    enum ctrl_code current_op = dev_ctrl.op_buffer[dev_ctrl.next_op_out];
    void *current_op_params = dev_ctrl.op_params_buffer[dev_ctrl.next_op_out];
    dev_ctrl.op_buffer[dev_ctrl.next_op_out] = CQ_CTRL_IDLE;
    dev_ctrl.op_params_buffer[dev_ctrl.next_op_out] = NULL;
    --dev_ctrl.num_ops;
    ++dev_ctrl.next_op_out;
    dev_ctrl.next_op_out %= __CQ_DEVICE_QUEUE_SIZE__;
    pthread_cond_signal(&dev_ctrl.cond_queue_full);
    pthread_mutex_unlock(&dev_ctrl.device_lock);

    cq_status status = control_registry[current_op](current_op_params);

    pthread_mutex_lock(&dev_ctrl.device_lock);
    if (current_op == CQ_CTRL_INIT || current_op == CQ_CTRL_FINALISE)
      dev_ctrl.lifecycle_status = status;
  }
  dev_ctrl.device_busy = false;
  pthread_cond_broadcast(&dev_ctrl.cond_device_busy);
  pthread_mutex_unlock(&dev_ctrl.device_lock);
  return NULL;
}

int finalise_device(const unsigned int VERBOSITY) {
  if (!dev_ctrl.worker_started)
    return CQ_WARNING;
  host_wait_all_ops();
  if (VERBOSITY > 0)
    printf("Finalising device.\n");

  unsigned int verbosity = VERBOSITY;
  host_send_ctrl_op(CQ_CTRL_FINALISE, &verbosity);
  host_wait_all_ops();
  cq_status status = dev_ctrl.lifecycle_status;
  stop_device();
  return status;
}
