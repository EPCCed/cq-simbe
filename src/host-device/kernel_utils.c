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

static pthread_mutex_t registry_lock = PTHREAD_MUTEX_INITIALIZER;

cq_status register_qkern(qkern kernel) {
  qkern_map candidate = {0};
  cq_status status = CQ_ERROR;
  if (kernel && kernel(0, NULL, 0, NULL, &candidate) == CQ_SUCCESS && candidate.fname[0]) {
    candidate.fn = kernel;
    status = CQ_SUCCESS;
  }
  pthread_mutex_lock(&registry_lock);
  if (status == CQ_SUCCESS) {
    for (size_t i=0; i<qk_reg.next_available_slot; ++i) {
      if (qk_reg.qkernels[i].fn == kernel) {
        status = CQ_WARNING;
        break;
      }
      if (!strcmp(qk_reg.qkernels[i].fname, candidate.fname)) {
        status = CQ_ERROR;
        break;
      }
    }
  }
  if (status == CQ_SUCCESS && qk_reg.next_available_slot == __CQ_MAX_NUM_QKERN__)
    status = CQ_ERROR;
  cq_status agreed = agree_kernel_registration(candidate.fname, status, 0);
  if (agreed != CQ_ERROR && status == CQ_SUCCESS)
    qk_reg.qkernels[qk_reg.next_available_slot++] = candidate;
  pthread_mutex_unlock(&registry_lock);
  return agreed;
}

cq_status register_pqkern(pqkern kernel) {
  pqkern_map candidate = {0};
  cq_status status = CQ_ERROR;
  if (kernel && kernel(0, NULL, 0, NULL, NULL, &candidate) == CQ_SUCCESS && candidate.fname[0]) {
    candidate.fn = kernel;
    status = CQ_SUCCESS;
  }
  pthread_mutex_lock(&registry_lock);
  if (status == CQ_SUCCESS) {
    for (size_t i=0; i<pqk_reg.next_available_slot; ++i) {
      if (pqk_reg.pqkernels[i].fn == kernel) {
        status = CQ_WARNING;
        break;
      }
      if (!strcmp(pqk_reg.pqkernels[i].fname, candidate.fname)) {
        status = CQ_ERROR;
        break;
      }
    }
  }
  if (status == CQ_SUCCESS && pqk_reg.next_available_slot == __CQ_MAX_NUM_QKERN__)
    status = CQ_ERROR;
  cq_status agreed = agree_kernel_registration(candidate.fname, status, 1);
  if (agreed != CQ_ERROR && status == CQ_SUCCESS)
    pqk_reg.pqkernels[pqk_reg.next_available_slot++] = candidate;
  pthread_mutex_unlock(&registry_lock);
  return agreed;
}

cq_status find_qkern_pointer(char const * const FNAME, qkern * qk) {
  pthread_mutex_lock(&registry_lock);
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

  pthread_mutex_unlock(&registry_lock);
  return status;
}

cq_status find_qkern_name(const qkern QK, char ** fname) {
  pthread_mutex_lock(&registry_lock);
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

  pthread_mutex_unlock(&registry_lock);
  return status;
}

cq_status find_pqkern_pointer(char const * const FNAME, pqkern * pqk) {
  pthread_mutex_lock(&registry_lock);
  *pqk = NULL;
  int status = CQ_SUCCESS;

  for (size_t i = 0; i < pqk_reg.next_available_slot; ++i) {
    if (!strcmp(FNAME, pqk_reg.pqkernels[i].fname)) {
      // We found it!
      *pqk = pqk_reg.pqkernels[i].fn;
      break;
    }
  }

  if (*pqk == NULL) status = CQ_ERROR;

  pthread_mutex_unlock(&registry_lock);
  return status;
}

cq_status find_pqkern_name(pqkern const PQK, char ** fname) {
  pthread_mutex_lock(&registry_lock);
  *fname = NULL;
  int status = CQ_SUCCESS;

  for (size_t i = 0; i < pqk_reg.next_available_slot; ++i) {
    if (PQK == pqk_reg.pqkernels[i].fn) {
      // We found it!
      *fname = pqk_reg.pqkernels[i].fname;
      break;
    }
  }

  if (*fname == NULL) status = CQ_ERROR;

  pthread_mutex_unlock(&registry_lock);
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
    size_t strsz = strlen(FNAME) + 1;
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

cq_status fort_create_exec_handle(cq_exec **ehp) {
  if (!ehp || *ehp) return CQ_ERROR;
  *ehp = calloc(1, sizeof(**ehp));
  if (!*ehp) return CQ_ERROR;
  /* No pthread primitives exist yet. Distinguish an empty handle from a
     successfully submitted zero-shot execution, which wait accepts. */
  (*ehp)->id = (size_t)-1;
  (*ehp)->expected_shots = (size_t)-1;
  (*ehp)->status = CQ_ERROR;
  return CQ_SUCCESS;
}

cq_status fort_free_exec_handle(cq_exec ** ehp) {
  if (!ehp || exec_is_live(*ehp)) return CQ_ERROR;
  free(*ehp);
  *ehp = NULL;
  return CQ_SUCCESS;
}
