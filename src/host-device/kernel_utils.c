#include <string.h>
#include "kernel_utils.h"

struct qkern_registry qk_reg;
struct pqkern_registry pqk_reg;

cq_status register_qkern(qkern kernel) {
  cq_status status = CQ_ERROR;
  const char * fname;
  
  if (kernel != NULL && qk_reg.next_available_slot < __MAX_NUM_QKERN__) {
    status = find_qkern_name(kernel, &fname);
    if (status == CQ_SUCCESS) {
      // This kernel has already been registered!
      status = CQ_WARNING;
    } else {
      qkern_map * pkmap = &qk_reg.qkernels[qk_reg.next_available_slot];
      kernel(0, NULL, NULL, pkmap);
      if (pkmap->fname[0] != '\0') {
        pkmap->fn = kernel;
        ++qk_reg.next_available_slot;
        status = CQ_SUCCESS;
      } else {
        status = CQ_ERROR;
      }
    }
  }

  return status;
}

cq_status register_pqkern(pqkern kernel) {
  return CQ_ERROR;
}

cq_status find_qkern_pointer(char const * const FNAME, qkern * qk) {
  *qk = NULL;
  int status = CQ_SUCCESS;
  
  for (size_t i = 0; i < qk_reg.next_available_slot; ++i) {
    if (!strcmp(FNAME, qk_reg.qkernels[i].fname)) {
      // We found it!
      *qk = qk_reg.qkernels[i].fn;
      break;
    }
  }

  if (*qk == NULL) status = CQ_ERROR;

  return status;
}

cq_status find_qkern_name(const qkern QK, char const ** fname) {
  *fname = NULL;
  cq_status status = CQ_SUCCESS;

  for (size_t i = 0; i < qk_reg.next_available_slot; ++i) {
    if (QK == qk_reg.qkernels[i].fn) {
      // We found it!
      *fname = qk_reg.qkernels[i].fname;
      break;
    }
  }

  if (*fname == NULL) status = CQ_ERROR;

  return status;
}

cq_status find_pqkern_pointer(char const * const FNAME, pqkern * pqk) {
  *pqk = NULL;
  int status = CQ_SUCCESS;
  
  for (size_t i = 0; i < qk_reg.next_available_slot; ++i) {
    if (!strcmp(FNAME, pqk_reg.pqkernels[i].fname)) {
      // We found it!
      *pqk = pqk_reg.pqkernels[i].fn;
      break;
    }
  }

  if (*pqk == NULL) status = CQ_ERROR;

  return status;
}

cq_status find_pqkern_name(pqkern const PQK, char const ** fname) {
  *fname = NULL;
  int status = CQ_SUCCESS;

  for (size_t i = 0; i < qk_reg.next_available_slot; ++i) {
    if (PQK == pqk_reg.pqkernels[i].fn) {
      // We found it!
      *fname = pqk_reg.pqkernels[i].fname;
      break;
    }
  }

  if (*fname == NULL) status = CQ_ERROR;

  return status;
}