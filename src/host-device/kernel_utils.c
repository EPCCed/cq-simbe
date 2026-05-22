#include <stdbool.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include "kernel_utils.h"

//#include "mpi_comms.h"
#include "src/host-device/comms.h"
#include <stdio.h>

struct qkern_registry qk_reg;
struct pqkern_registry pqk_reg;

cq_status register_qkern(qkern kernel) {
  cq_status status = CQ_ERROR;
  char * fname;
  
  if (kernel != NULL && qk_reg.next_available_slot < __CQ_MAX_NUM_QKERN__) {
    status = find_qkern_name(kernel, &fname);
    if (status == CQ_SUCCESS) {
      // This kernel has already been registered!
      status = CQ_WARNING;
    } else {
      qkern_map * pkmap = &qk_reg.qkernels[qk_reg.next_available_slot];
      kernel(0, NULL, 0, NULL, pkmap);
      if (pkmap->fname[0] != '\0') {
        pkmap->fn = kernel;
        ++qk_reg.next_available_slot;
        status = CQ_SUCCESS;
      } else {
        status = CQ_ERROR;
      }
    }
  }

  //host_device_sync_comms();
  RUN_HOST_ONLY();
  host_wait_all_ops();
  return status;
}

cq_status register_pqkern(pqkern kernel) {
  return CQ_ERROR;
}

cq_status find_qkern_pointer(char const * const FNAME, qkern * qk) {
  *qk = NULL;
  cq_status status = CQ_SUCCESS;
  
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

cq_status find_qkern_name(const qkern QK, char ** fname) {
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

cq_status find_pqkern_name(pqkern const PQK, char ** fname) {
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

void init_exec_handle(const size_t NQUBITS, const size_t NSHOTS, const size_t NMEASURE, cq_exec * ehp) {
  ehp->id = assign_exec_id();
  ehp->exec_init = true;
  ehp->complete = false;
  ehp->halt = false;
  ehp->status = CQ_ERROR;
  ehp->nqubits = NQUBITS;
  ehp->completed_shots = 0;
  ehp->expected_shots = NSHOTS;
  ehp->nmeasure = NMEASURE;
  ehp->params_size = 0;
  ehp->fname = NULL;
  ehp->qreg = NULL;
  ehp->creg = NULL;
  ehp->params = NULL;
  pthread_mutex_init(&(ehp->lock), NULL);
  pthread_cond_init(&(ehp->cond_exec_complete), NULL);
  return;
}

void finalise_exec_handle(cq_exec * ehp) {
  if (ehp->exec_init) {
    pthread_mutex_lock(&(ehp->lock));
    ehp->exec_init = false;
    pthread_mutex_unlock(&(ehp->lock));
    pthread_mutex_destroy(&(ehp->lock));
    pthread_cond_destroy(&(ehp->cond_exec_complete));
  }
  return;
}

// fortran helpers -- just don't call them from C
cq_status fort_insert_to_qkern_map(const char * FNAME, qkern_map * reg) {
  if (reg != NULL) {
    size_t strsz = sizeof(FNAME);
    if (strsz < __CQ_MAX_QKERN_NAME_LENGTH__) {
      strcpy(reg->fname, FNAME);
      return CQ_SUCCESS;
    } else {
      reg->fname[0] = '\0';
      return CQ_ERROR;
    }
  }
  return CQ_ERROR;
}

cq_status fort_create_exec_handle(cq_exec ** ehp) {
  cq_status status = CQ_ERROR;
  if (*ehp == NULL) {
    *ehp = (cq_exec *) malloc(sizeof(cq_exec));
    // check malloc
    if (*ehp != NULL) status = CQ_SUCCESS;
  }
  return status;
}

cq_status fort_free_exec_handle(cq_exec ** ehp) {
  free(*ehp);
  *ehp = NULL;
  return CQ_SUCCESS;
}
