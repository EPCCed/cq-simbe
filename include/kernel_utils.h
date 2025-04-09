#ifndef CQ_KERNEL_UTILS_H
#define CQ_KERNEL_UTILS_H

#include <string.h>
#include "datatypes.h"

// This has to be a macro as we need __func__ to return the
// name of the user function, not our own cq_register_kernel
// function
#define CQ_REGISTER_KERNEL(reg) \
if (reg != NULL) {\
  size_t strsz = sizeof(__func__);\
  if (strsz < __CQ_MAX_QKERN_NAME_LENGTH__) {\
    strcpy(reg->fname, __func__);\
    return;\
  } else {\
    reg->fname[0] = '\0';\
  }\
}

struct qkern_registry {
  struct qkern_map qkernels[__CQ_MAX_NUM_QKERN__];
  size_t next_available_slot;
};

struct pqkern_registry {
  struct pqkern_map pqkernels[__CQ_MAX_NUM_QKERN__];
  size_t next_available_slot;
};

extern struct qkern_registry qk_reg;
extern struct pqkern_registry pqk_reg;

typedef struct qkern_params {
  const char * FNAME;
  const size_t NQUBITS;
  qubit * qreg;
  cstate * creg;
  void * params;
} qkern_params;

// Kernel registration

cq_status register_qkern(qkern kernel);

cq_status register_pqkern(pqkern kernel);

// find user kernels

cq_status find_qkern_pointer(const char * FNAME, qkern * qk);

cq_status find_qkern_name(qkern const QK, char const ** fname);

cq_status find_pqkern_pointer(const char * FNAME, pqkern * pqk);

cq_status find_pqkern_name(pqkern const PQK, char const ** fname);

#endif