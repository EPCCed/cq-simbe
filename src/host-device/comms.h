#ifndef CQ_HOST_DEVICE_COMMS_H
#define CQ_HOST_DEVICE_COMMS_H

#include <pthread.h>
#include <stdbool.h>
#include "datatypes.h"
#include "src/host/opcodes.h"

#define __CQ_DEVICE_QUEUE_SIZE__ 16

#define RUN_HOST_ONLY()  \
  {                      \
    if (is_device()) {   \
      return CQ_SUCCESS; \
    }                    \
  }

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
  void* op_params_buffer[__CQ_DEVICE_QUEUE_SIZE__];
};

typedef struct device_alloc_params {
  const size_t NQUBITS;
  size_t qregistry_idx;
  cq_status status;
} device_alloc_params;

extern struct dev_link dev_ctrl;

// TODO: pth as in original comms.c
int initialise_device(const unsigned int VERBOSITY);

// TODO: pth wrapper around insert_op
size_t host_send_ctrl_op(const enum ctrl_code OP, void* ctrl_params);

// TODO: pth wrapper around device_wait_all_ops
size_t host_wait_all_ops(void);

size_t device_sync_exec(const cq_status STATUS,
                        const size_t SHOT,
                        cstate const* const RESULT,
                        cq_exec* ehp);

// TODO: Implement for pthreads
void host_device_sync_comms(void);

// TODO: pth as in original comms.c
int finalise_device(const unsigned int VERBOSITY);

/// generates and assigns id to the executor that is used for host-device
/// communication.
/// @return new executor id
size_t assign_exec_id(void);

///
/// check if MPI process is assigned to the device.
/// @return true if the MPI process is device process. false by default (when CQ
/// built without MPI)
bool is_device(void);

///
/// initialises correct QuEST environment depending on the build opotions.
void init_quest_env(void);

#endif
