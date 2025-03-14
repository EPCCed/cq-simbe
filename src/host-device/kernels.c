#include <string.h>
#include "kernels.h"

#ifndef NDEBUG
#include <stddef.h>
#include <stdio.h>
#endif

struct qkern_registry qk_reg;
struct pqkern_registry pqk_reg;

int register_qkern(qkern kernel) {
  qkern_map * pkmap = &qk_reg.qkernels[qk_reg.next_available_slot];
  pkmap->fn = kernel;
  kernel(0, NULL, NULL, pkmap);
  ++qk_reg.next_available_slot;

#ifndef NDEBUG
  size_t slot = qk_reg.next_available_slot - 1;
  printf("Registered kernel function %s (%p) in slot %lu.\n", 
    qk_reg.qkernels[slot].fname, qk_reg.qkernels[slot].fn, slot);
#endif
  
  return 0;
}

int register_pqkern(pqkern kernel) {
  return 0;
}

int find_qkern_pointer(char const * const FNAME, qkern * qk) {
  *qk = NULL;
  int status = 0;
  
  for (size_t i = 0; i < qk_reg.next_available_slot; ++i) {
    if (!strcmp(FNAME, qk_reg.qkernels[i].fname)) {
      // We found it!
      *qk = qk_reg.qkernels[i].fn;
      break;
    }
  }

  if (*qk == NULL) status = -1;

  return status;
}

int find_qkern_name(const qkern QK, char const ** fname) {
  *fname = NULL;
  int status = 0;

  for (size_t i = 0; i < qk_reg.next_available_slot; ++i) {
    if (QK == qk_reg.qkernels[i].fn) {
      // We found it!
      *fname = qk_reg.qkernels[i].fname;
      break;
    }
  }

  if (*fname == NULL) status = -1;

  return status;
}

int find_pqkern_pointer(char const * const FNAME, pqkern * pqk) {
  *pqk = NULL;
  int status = 0;
  
  for (size_t i = 0; i < qk_reg.next_available_slot; ++i) {
    if (!strcmp(FNAME, pqk_reg.pqkernels[i].fname)) {
      // We found it!
      *pqk = pqk_reg.pqkernels[i].fn;
      break;
    }
  }

  if (*pqk == NULL) status = -1;

  return status;
}

int find_pqkern_name(pqkern const PQK, char const ** fname) {
  *fname = NULL;
  int status = 0;

  for (size_t i = 0; i < qk_reg.next_available_slot; ++i) {
    if (PQK == pqk_reg.pqkernels[i].fn) {
      // We found it!
      *fname = pqk_reg.pqkernels[i].fname;
      break;
    }
  }

  if (*fname == NULL) status = -1;

  return status;
}