#ifndef CQ_HOST_DEVICE_COMMS_H
#define CQ_HOST_DEVICE_COMMS_H

#include "comms_utils.h"
#include "datatypes.h"
#include "src/host/opcodes.h"

#include <pthread.h>
#include <stdbool.h>

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
  void * op_params_buffer[__CQ_DEVICE_QUEUE_SIZE__];
};

typedef struct device_alloc_params {
  const size_t NQUBITS;
  size_t qregistry_idx;
  cq_status status;
} device_alloc_params;

extern struct dev_link dev_ctrl;

///
/// initialises the device.
/// @param VERBOSITY unsigned integer controlling diagnostic output
/// @return TODO: always returns 0
int initialise_device(const unsigned int VERBOSITY);

#if CQ_WITH_MPI_COMMS
#include <mpi.h>

///
/// initialises the device with user-provided MPI communicator.
/// @param cq_comm MPI (sub)communicator provided by the user
/// @param VERBOSITY unsigned integer controlling diagnostic output
/// @return TODO: always returns 0
int initialise_device_with_custom_mpi_comm(MPI_Comm cq_comm,
                                           const unsigned int VERBOSITY);
#endif

///
/// sends control operation from host to the device.
/// @param OP an enum argument specifying CQ_CTRL_OP
/// @param[in,out] ctrl_params void pointer to arbitrary params associated
/// with control operation
/// @return number of control operation in device queue
size_t host_send_ctrl_op(const enum ctrl_code OP, void * ctrl_params);

///
/// blocks host and waits for the device to finish operation.
/// @return number of operations, which should always be 0
size_t host_wait_all_ops(void);

///
/// syncrhonises the state of device with the executor.
/// @param STATUS current device status
/// @param SHOT current shot id
/// @param RESULT classical buffer storing results from measurements
/// collected up to this point
/// @param ehp pointer to the executor handle
size_t device_sync_exec(const cq_status STATUS,
                        const size_t SHOT,
                        cstate const * const RESULT,
                        cq_exec * ehp);

///
/// synchronises host-device communication thread.
void host_device_sync_comms(void);

///
/// finalises the device.
/// @param VERBOSITY unsigned integer controlling diagnostic output
/// @return TODO: always returns 0
int finalise_device(const unsigned int VERBOSITY);

/// generates and assigns id to the executor that is used for host-device
/// communication.
/// @return new executor id
size_t assign_exec_id(void);

///
/// initialises correct QuEST environment depending on the build opotions.
void init_quest_env(void);

#endif
