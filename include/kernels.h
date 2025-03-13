#ifndef CQ_KERNELS_H
#define CQ_KERNELS_H

#include "datatypes.h"

// This has to be a macro as we need __func__ to return the
// name of the user function, not our own cq_register_kernel
// function
#define CQ_REGISTER_KERNEL(reg) \
if (reg != NULL) {\
  strcpy(reg->fname, __func__);\
  return;\
}

struct qkern_registry {
  struct qkern_map qkernels[__MAX_NUM_QKERN__];
  size_t next_available_slot;
};

extern struct qkern_registry qk_reg;

// Kernel registration

int register_qkern(qkern kernel);

int register_pqkern(pqkern kernel);

#endif