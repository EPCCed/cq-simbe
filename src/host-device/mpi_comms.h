#ifndef CQ_HOST_DEVICE_MPI_COMMS_H
#define CQ_HOST_DEVICE_MPI_COMMS_H

#include <stddef.h>
#include "datatypes.h"
#include "src/host-device/comms.h"
#include "src/host/opcodes.h"

#include <mpi.h>

// ----------------------------------------------------------------------------
// Macros
// ----------------------------------------------------------------------------
#define CQ_MPI_IMPL_DEBUG

#define CQ_MPI_HOST_RANK 0
#define CQ_MPI_DEVICE_RANK 1
#define CQ_MPI_DEVICE_MASTER_RANK 0
#define CQ_MPI_COMMS_TAG 0

#define CQ_MPI_RUNTIME_ERROR -4
#define CQ_MPI_MALLOC_ERROR -5

// device_wait_comms();
// if (is_quantum_worker()) {
//      return CQ_SUCCESS;
//    }

#define RUN_HOST_ONLY()                   \
  {                                       \
    int rank = get_rank();                \
    if (rank == -1) {                     \
      return CQ_ERROR;                    \
    }                                     \
    if (get_rank() != CQ_MPI_HOST_RANK) { \
      return CQ_SUCCESS;                  \
    }                                     \
  }

///
/// initialises MPI environment both on host and on the device.
/// @param VERBOSITY unsigned integer controlling diagnostic output.
void init_host_device_mpi(const unsigned int VERBOSITY);

///
/// finalises MPI environment both on host and on the device.
/// @param VERBOSITY unsigned integer controlling diagnostic output.
void finalise_host_device_mpi(const unsigned int VERBOSITY);

// ----------------------------------------------------------------------------
// Host Comm Ops
// ----------------------------------------------------------------------------

///
/// sends the control operation to the device.
/// @param OP an enum argument specifying CQ_CTRL_OP
/// @param params datatype argument storing arbitrary data and data type, useful
/// for a given control operation
// void mpi_host_send_ctrl_op(const enum ctrl_code OP, struct ctrl_params*
// params);

///
/// sends the control operation to the device.
/// @param OP an enum argument specifying CQ_CTRL_OP
/// @param[in,out] params void pointer to arbitrary control parameters
void mpi_host_send_ctrl_op(const enum ctrl_code OP, void* params);

///
/// blocks and waits for the operations to complete on the device.
/// at the end it should recieve updated params if needed???
void mpi_host_wait_all_ops(void);

///
/// wrapper around device_wait_comms, intended to be called from the host.
/// The intended caller of this function is cq_finalise only!
void host_device_final_sync(void);
// TODO: rename above with below
void host_device_sync_comms(void);

// ----------------------------------------------------------------------------
// Device Comm Ops
// ----------------------------------------------------------------------------

///
/// wait for message with control operation from the host. The intended use of
/// this function is to use it as a listener in the loop on the device.
// void mpi_device_recv_ctrl_op(void);

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
void* device_listen(void* args);

///
/// dispatches recieved control operation to the worker thread.
/// @param OP an enum argument specifying CQ_CTRL_OP
void device_dispatch_ctrl_op(const enum ctrl_code OP);

///
/// blocks device main thread and awaits for the worker to complete.
size_t device_wait_all_ops(void);

///
/// blocks device master thread and awaits for the comms to complete.
/// it needs to be called only once at the end to ensure that the main thread
/// does not clean-up and close MPI before work is done on the worker.
void device_wait_comms(void);
// TODO: above can be static and in mpi_comms.c

// ----------------------------------------------------------------------------
// Device Control Paramaters Comms
// ----------------------------------------------------------------------------

///
/// communicates control parameters between host and device.
/// @param OP an enum argument specifying CQ_CTRL_OP
/// @param[in,out] params void pointer to arbitrary params
void host_comm_params(const enum ctrl_code OP, void* params);

// NOTE: Alloc params can check the results across thread
// before Device Master sends to Host (in send)
// in recv Master will broadcast to Q-workers
///
/// receives allocation parameters from the source.
/// @param[out] params reference to parameters to store the results of
/// communication
/// @param src source rank of incoming message
void recv_alloc_params(device_alloc_params* params, int src);

///
/// sends allocation parameters to the destination.
/// @param[in] params reference to parameters to communicate
/// @param dest destination rank of outgoing message
void send_alloc_params(const device_alloc_params* params, int dest);

size_t recv_exec_id(const int src);
void send_exec_id(const size_t id, const int dest);

// NOTE: Exec params can be send by Device Master to Host (in send)
// in recv Master will broadcast to Q-workers
///
/// receives executor handle from the source. Also, if called on device,
/// allocates executor in on-device memory, which then needs to be freed using
/// device_free_exec.
/// @param[out] ehp executor handle used for host-device offloading
/// @param src source rank of incoming message
void recv_exec_params(cq_exec** ehp, int src);

/// sends updated executor handle to the destination.
/// @param[in] ehp executor handle used for host-device offloading
/// @param dest destination rank of outgoing message
void send_exec_params(cq_exec* ehp, int dest);

///
/// frees on-device memory pointed to by executor handle.
/// @param[in,out] ehp executor handle used for host-device offloading
void device_free_exec(cq_exec** ehp);

// ----------------------------------------------------------------------------
// Helpers
// ----------------------------------------------------------------------------

///
/// returns the name of either host or the device. Used for printing
/// diagnostics.
/// @return name of the device (or host).
const char* get_comm_source(void);

///
/// returns the current MPI rank.
/// @return MPI rank of a given process.
int get_rank(void);

///
/// converts CQ control code to string.
/// @param OP an enum argument specifying CQ_CTRL_OP
/// @return string represntation of OP.
const char* op_to_str(const enum ctrl_code OP);

///
/// prints values of the alloc parameters.
/// @param[in] params reference to parameters
void print_alloc_params(const device_alloc_params* params);

///
/// prints members of the executor.
/// @param[in] ehp reference to executor
void print_ehp(const cq_exec* ehp);

size_t assign_exec_id(void);

int is_quantum_worker(void);
MPI_Comm get_quest_comm(void);

// ----------------------------------------------------------------------------
// TODO: Need to go to comms.h and comms.c and use existing stuff to wrap around
// ----------------------------------------------------------------------------

size_t comms_exec_sync(cq_exec* const ehp);
size_t comms_exec_wait(cq_exec* const ehp);
void comms_exec_halt(cq_exec* const ehp);

#endif
