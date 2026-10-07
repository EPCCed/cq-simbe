#ifndef CQ_HOST_DEVICE_COMMS_MPI_H
#define CQ_HOST_DEVICE_COMMS_MPI_H

#include "datatypes.h"
#include "src/host-device/comms.h"
#include "src/host/opcodes.h"

#include <mpi.h>

#include <stdbool.h>
#include <stddef.h>

///
/// initialises MPI environment both on host and on the device.
/// @param VERBOSITY unsigned integer controlling diagnostic output.
void init_host_device_mpi(const unsigned int VERBOSITY);

///
/// finalises MPI environment both on host and on the device.
/// @param VERBOSITY unsigned integer controlling diagnostic output.
void finalise_host_device_mpi(const unsigned int VERBOSITY);

// ----------------------------------------------------------------------------
// Device Comm Ops
// ----------------------------------------------------------------------------

///
/// initialises on-device communication thread responsible for MPI messaging
/// with host.
/// @param VERBOSITY unsigned integer controlling diagnostic output.
void device_init_comms(const unsigned int VERBOSITY);

///
/// finalises on-device communication thread responsible for MPI messaging
/// with host.
/// @param VERBOSITY unsigned integer controlling diagnostic output.
void device_finalise_comms(const unsigned int VERBOSITY);

///
/// starts listening for the incoming messages from the host.
/// @param args arbitrary arguments to function to satisfy pthread function
/// signature
/// @return arbitrary return data to satisfy pthread function signature.
/// Currently always returns NULL.
void * device_listen(void * args);

///
/// dispatches recieved control operation to the worker thread.
/// @param OP an enum argument specifying CQ_CTRL_OP
void device_dispatch_ctrl_op(const enum ctrl_code OP);

///
/// blocks device master thread and awaits for the comms to complete.
/// it needs to be called only once at the end to ensure that the main thread
/// does not clean-up and close MPI before work is done on the worker.
void device_wait_comms(void);

// ----------------------------------------------------------------------------
// Device Control Paramaters Comms
// ----------------------------------------------------------------------------

///
/// communicates control parameters between host and device.
/// @param OP an enum argument specifying CQ_CTRL_OP
/// @param[in,out] params void pointer to arbitrary params
void host_comm_params(const enum ctrl_code OP, void * params);

///
/// receives allocation parameters from the source.
/// @param[out] params reference to parameters to store the results of
/// communication
/// @param src source rank of incoming message
void recv_alloc_params(device_alloc_params * params, const int src);

///
/// sends allocation parameters to the destination.
/// @param[in] params reference to parameters to communicate
/// @param dest destination rank of outgoing message
void send_alloc_params(const device_alloc_params * params, const int dest);

///
/// receives executor id used for matching results with correct executor.
/// @param src source rank of incoming message
/// @return executor id
size_t recv_exec_id(const int src);

///
/// sends executor id used for matching results with correct executor.
/// @param id executor id to be sent
/// @param dest destination rank of outgoing message
void send_exec_id(const size_t id, const int dest);

///
/// receives executor handle from the source. Also, if called on device,
/// allocates executor in on-device memory, which then needs to be freed using
/// device_free_exec.
/// @param[out] ehp executor handle used for host-device offloading
/// @param src source rank of incoming message
void recv_exec_params(cq_exec ** ehp, const int src);

/// sends updated executor handle to the destination.
/// @param[in] ehp executor handle used for host-device offloading
/// @param dest destination rank of outgoing message
void send_exec_params(cq_exec * ehp, const int dest);

///
/// frees on-device memory pointed to by executor handle.
/// @param[in,out] ehp executor handle used for host-device offloading
void device_free_exec(cq_exec ** ehp);

// ----------------------------------------------------------------------------
// Helpers
// ----------------------------------------------------------------------------

///
/// returns the name of either host or the device. Used for printing
/// diagnostics.
/// @return name of the device (or host).
const char * get_comm_source(void);

///
/// returns the current MPI rank.
/// @return MPI rank of a given process.
int get_rank(void);

///
/// converts CQ control code to string.
/// @param OP an enum argument specifying CQ_CTRL_OP
/// @return string representation of OP.
const char * op_to_str(const enum ctrl_code OP);

///
/// prints values of the alloc parameters.
/// @param[in] params reference to parameters
void print_alloc_params(const device_alloc_params * params);

///
/// prints members of the executor.
/// @param[in] ehp reference to executor
void print_ehp(const cq_exec * ehp);

///
/// check if MPI process is a quantum worker (not resposible for host-device
/// communication).
/// @return true if the MPI process is assigned to QuEST simulation.
bool is_quantum_worker(void);

/// validates the number of MPI processes to meet QuEST constraints.
/// Exits program if check failed.
/// @param nproc number of MPI processes
void validate_nproc(const int nproc);

#endif
