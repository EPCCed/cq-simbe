#ifndef HOST_DEVICE_MPI_COMMS_H
#define HOST_DEVICE_MPI_COMMS_H

#include "datatypes.h"
#include "src/host/opcodes.h"

#include <mpi.h>

#include <pthread.h>
#include <stdbool.h>

#define __CQ_DEVICE_QUEUE_SIZE__ 16
#define __CQ_HOST_DEVICE_MPI_PROC__ 0
#define CQ_MPI_HOST_RANK 0
#define CQ_HOST_DEVICE_MPI_TAG 0

enum ctrl_params_datatype { PARAMS_UINT_T, PARAMS_ALLOC_T, PARAMS_EXEC_T };

struct ctrl_params_header {
  enum ctrl_params_datatype type;
  int params_msg_size;
};

struct device_ctrl_params {
  enum ctrl_params_datatype type;
  void* data;
};

void print_params_header(struct ctrl_params_header header);

typedef void (*param_packer_fn)(void* src, void* dest);

void pack_uint_params(void* src, void* dest);
void pack_alloc_params(void* src, void* dest);
void pack_exec_params(void* src, void* dest);
// or send_xxx_params and recv_xxx_params
// void send_uint_params(void* params);
// void send_alloc_params(void* params);
// void send_exec_params(void* params);

void send_uint_params(struct device_ctrl_params ctrl_params);
void send_alloc_params(struct device_ctrl_params ctrl_params);
void send_exec_params(struct device_ctrl_params ctrl_params);

void recv_uint_params(void* params);
void recv_alloc_params(void* params);
void recv_exec_params(void* params);

// void comm_ctrl_params(struct ctrl_params_header msg_header,
//                       void* params,
//                       param_packer_fn packer);
//  void send_ctrl_params(struct ctrl_params_header msg_header,
//                        void* params,
//                        param_packer_fn packer);
void send_ctrl_params(struct device_ctrl_params ctrl_params);

void recv_ctrl_params(void* params);

int mpi_initialise_device(const unsigned int VERBOSITY);

void mpi_host_comm_ctrl_op(const enum ctrl_code OP,
                           struct device_ctrl_params ctrl_params);

size_t mpi_host_send_ctrl_op(const enum ctrl_code OP,
                             struct device_ctrl_params ctrl_params);

size_t mpi_host_recv_ctrl_op(void);

size_t mpi_host_sync_exec(cq_exec* const ehp);

size_t mpi_host_wait_exec(cq_exec* const ehp);

void mpi_host_request_halt(cq_exec* const ehp);

size_t mpi_host_wait_all_ops(void);

size_t mpi_device_sync_exec(const cq_status STATUS,
                            const size_t SHOT,
                            cstate const* const RESULT,
                            cq_exec* ehp);

void* mpi_device_control_thread(void*);

int mpi_finalise_device(const unsigned int VERBOSITY);

#endif
