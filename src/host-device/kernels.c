#include "kernels.h"

#ifndef NDEBUG
#include <stddef.h>
#include <stdio.h>
#endif

struct qkern_registry qk_reg;

int register_qkern(qkern kernel) {
  qkern_map * pkmap = &qk_reg.qkernels[qk_reg.next_available_slot];
  kernel(0, NULL, NULL, pkmap);
  ++qk_reg.next_available_slot;

#ifndef NDEBUG
  size_t slot = qk_reg.next_available_slot - 1;
  printf("Registered kernel function %s in slot %lu.\n", 
    qk_reg.qkernels[slot].fname, slot);
#endif
  
  return 0;
}

int register_pqkern(pqkern kernel) {
  return 0;
}