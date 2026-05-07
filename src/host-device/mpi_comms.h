#ifndef CQ_HOST_DEVICE_MPI_COMMS_H
#define CQ_HOST_DEVICE_MPI_COMMS_H

#include <stddef.h>
#include "src/host-device/comms.h"
#include "src/host/opcodes.h"

// ----------------------------------------------------------------------------
// Macros
// ----------------------------------------------------------------------------
#define CQ_MPI_HOST_RANK 0
#define CQ_MPI_DEVICE_RANK 1
#define CQ_MPI_COMMS_TAG 0

#define RUN_HOST_ONLY()                 \
  {                                     \
    int rank = get_rank();              \
    if (rank == -1)                     \
      return CQ_ERROR;                  \
    if (get_rank() != CQ_MPI_HOST_RANK) \
      return CQ_SUCCESS;                \
  }

// ----------------------------------------------------------------------------
// Datatypes
// ----------------------------------------------------------------------------

enum params_type {
  CQ_CTRL_PARAMS_UINT,  // this will not be needed
  CQ_CTRL_PARAMS_ALLOC,
  CQ_CTRL_PARAMS_EXEC
};

struct ctrl_params {
  enum params_type type;
  void* data;
};

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

// ----------------------------------------------------------------------------
// Device Comm Ops
// ----------------------------------------------------------------------------

///
/// wait for message with control operation from the host. The intended use of
/// this function is to use it as a listener in the loop on the device.
void mpi_device_recv_ctrl_op(void);

///
/// starts listening for the incoming messages from the host.
void device_listen(void);

///
/// dispatches recieved control operation to the worker thread.
/// @param OP an enum argument specifying CQ_CTRL_OP
void device_dispatch_ctrl_op(const enum ctrl_code OP);

///
/// blocks device comms thread and awaits for the worker to complete.
size_t device_wait_all_ops(void);

// ----------------------------------------------------------------------------
// Device Control Paramaters Comms
// ----------------------------------------------------------------------------

///
/// communicates control parameters between host and device.
/// @param OP an enum argument specifying CQ_CTRL_OP
/// @param[in,out] params void pointer to arbitrary params
void host_comm_params(const enum ctrl_code OP, void* params);

///
/// recieves allocation parameters from the source.
/// @param[out] params reference to parameters to store the results of
/// communication
/// @param src source rank of incoming message
void recv_alloc_params(device_alloc_params* params, int src);

///
/// sends allocation parameters to the destination.
/// @param[in] params reference to parameters to communicate
/// @param dest destination rank of outgoing message
void send_alloc_params(const device_alloc_params* params, int dest);

// void recv_exec_params(device_alloc_params* params, int src);
// void send_exec_params(device_alloc_params* params, int dest);

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

#endif
