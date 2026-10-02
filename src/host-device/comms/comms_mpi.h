#ifndef CQ_HOST_DEVICE_COMMS_MPI_H
#define CQ_HOST_DEVICE_COMMS_MPI_H

#include "datatypes.h"
#include "src/host-device/comms.h"
#include "src/host/opcodes.h"
#include <mpi.h>

void device_finalise_comms(unsigned int verbosity);
void *device_listen(void *args);
void device_dispatch_ctrl_op(enum ctrl_code op);
void device_wait_comms(void);
void host_comm_params(enum ctrl_code op, void *params);
void recv_alloc_params(device_alloc_params *params, int src);
void send_alloc_params(const device_alloc_params *params, int dest);
size_t recv_exec_id(int src);
void send_exec_id(size_t id, int dest);
void recv_exec_params(cq_exec **exec, int src);
void send_exec_params(cq_exec *exec, int dest);
void device_free_exec(cq_exec **exec);
const char *get_comm_source(void);
const char *op_to_str(enum ctrl_code op);
void print_alloc_params(const device_alloc_params *params);
void print_ehp(const cq_exec *exec);
bool is_quantum_worker(void);
#endif
