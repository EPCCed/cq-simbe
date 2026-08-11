#ifndef CQ_HOST_DEVICE_COMMS_CORE_H
#define CQ_HOST_DEVICE_COMMS_CORE_H

#include "datatypes.h"
#include "src/host/opcodes.h"

#include <stddef.h>

///
/// initialises device control thread, responsible for executing operations
/// recieved from the host.
/// @param VERBOSITY unsigned integer controlling diagnostic output.
/// @return TODO: always returns 0
int init_device_controls(const unsigned int VERBOSITY);

///
/// stops the device so that no further operations are executed.
void stop_device(void);

///
/// finalises device control thread, responsible for executing operations
/// recieved from the host.
/// @param VERBOSITY unsigned integer controlling diagnostic output.
/// @return TODO: always returns 0
int finalise_device_controls(const unsigned int VERBOSITY);

///
/// inserts control operation into device operation queue.
/// @param OP an enum argument specifying CQ_CTRL_OP
/// @param ctrl_param generic pointer to the parameters associated with OP
/// @return number of queued operations on the device.
size_t insert_op(const enum ctrl_code OP, void * ctrl_params);

///
/// synchronises the executor with the shots completed on the device.
/// @param ehp pointer to the executor handle
/// @return number of completed shots
size_t comms_exec_sync(cq_exec * const ehp);

///
/// waits for the device to complete work managed by the executor.
/// @param ehp pointer to the executor handle
/// @return number of completed shots
size_t comms_exec_wait(cq_exec * const ehp);

///
/// stops the execution of the kernel and updates the executor.
/// @param ehp pointer to the executor handle
void comms_exec_halt(cq_exec * const ehp);

///
/// waits for all operations to complete on the device.
/// @return number of operations, which should always be 0
size_t device_wait_all_ops(void);

///
/// routine to be executed by the control thread.
/// as long as the device is online, it waits for new control operations
/// and executes them in a loop.
/// @param par TODO: ???
/// @return TODO: ???
void * device_control_thread(void * par);

///
/// sends control operation from host to the device.
/// used in single process or non-MPI runs.
/// @param OP an enum argument specifying CQ_CTRL_OP
/// @param ctrl_param generic pointer to the parameters associated with OP
/// @return number of queued operations on the device.
size_t serial_host_send_ctrl_op(const enum ctrl_code OP, void * ctrl_params);

///
/// initialises the device.
/// used in single process or non-MPI runs.
/// @param VERBOSITY unsigned integer controlling diagnostic output
/// @return TODO: always returns 0
int serial_initialise_device(const unsigned int VERBOSITY);

///
/// blocks host and waits for the device to finish operation.
/// used in single process or non-MPI runs.
/// @return number of operations, which should always be 0
size_t serial_host_wait_all_ops(void);

///
/// synchronises host-device communication thread.
void serial_host_device_sync_comms(void);

///
/// finalises the device.
/// used in single process or non-MPI runs.
/// @param VERBOSITY unsigned integer controlling diagnostic output
/// @return TODO: always returns 0
int serial_finalise_device(const unsigned int VERBOSITY);

#endif
