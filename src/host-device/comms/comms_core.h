#ifndef CQ_HOST_DEVICE_COMMS_CORE_H
#define CQ_HOST_DEVICE_COMMS_CORE_H

#include "datatypes.h"
#include "src/host/opcodes.h"

#include <stddef.h>

int init_device_controls(const unsigned int VERBOSITY);

void stop_device(void);

int finalise_device_controls(const unsigned int VERBOSITY);

size_t insert_op(const enum ctrl_code OP, void* ctrl_params); 

size_t comms_exec_sync(cq_exec* const ehp);

size_t comms_exec_wait(cq_exec* const ehp);

void comms_exec_halt(cq_exec* const ehp);

size_t device_wait_all_ops(void);

void * device_control_thread(void *);
				      
#endif
