#include <stdlib.h>
#include "datatypes.h"
#include "host_ops.h"
#include "kernels.h"
#include "src/host-device/comms.h"

// Resource Management

cq_status alloc_qureg(qubit ** qrp, size_t N) {
  cq_status status = CQ_SUCCESS;
  
  // check qr is NULL
  // If it's not, we still allocate, but issue a CQ_WARNING.
  if (*qrp != NULL)  {
    status = CQ_WARNING;
  }
  
  device_alloc_params alloc_params = {
    .NQUBITS = N,
    .qregistry_idx = 0,
    .status = CQ_ERROR
  };

  *qrp = (qubit *) malloc(sizeof(qubit) * N);
  if (*qrp == NULL) {
    /*  whoops, the malloc failed
        originally we only malloc'd on the host after receiving
        CQ_SUCCESS from the device, but it's easier to undo
        this malloc in the event the device fails to allocate a register
        than it is to undo the allocation on the device if this fails
    */
    return CQ_ERROR;
  }
  
  host_send_ctrl_op(ALLOC, &alloc_params);
  host_wait_ctrl_op();

  if (alloc_params.status == CQ_SUCCESS) {
    for (size_t i = 0; i < N; ++i) {
      (*qrp)[i].registry_index = alloc_params.qregistry_idx;
      (*qrp)[i].offset = i;
      (*qrp)[i].N = N;
    }
  } else {
    // Something went wrong!
    // need to free qrp
    free(*qrp);
    *qrp = NULL;
    status = alloc_params.status;
  }

  return status;
}

cq_status free_qureg(qubit ** qrp) {
  if (*qrp == NULL) return CQ_WARNING;

  device_alloc_params dealloc_params = {
    .NQUBITS = 0,
    .qregistry_idx = (*qrp)[0].registry_index,
    .status = CQ_ERROR
  };

  host_send_ctrl_op(DEALLOC, &dealloc_params);
  host_wait_ctrl_op();

  if (dealloc_params.status == CQ_SUCCESS) {
    free(*qrp);
    *qrp = NULL;
  }

  return dealloc_params.status;
}

// Executors

cq_status sm_qrun(qkern kernel, qubit * qrp, const size_t NQUBITS, 
cstate * const crp, const size_t NMEASURE, const size_t NSHOTS) {
  cq_status status = CQ_ERROR;
  char const * fname = NULL;

  // proceed iff there are any shots
  // and the qreg is non-null
  // and either the creg is non-null or there will be no measurements anyway
  if (NSHOTS == 0) {
    status = CQ_SUCCESS;
  } else if (qrp != NULL && (NMEASURE == 0 || crp != NULL)) {
    status = find_qkern_name(kernel, &fname);    

    if (status == CQ_SUCCESS) {
      qkern_params qk_par = {
        .FNAME = fname,
        .NQUBITS = NQUBITS,
        .creg = crp,
        .qreg = qrp,
        .params = NULL
      };

      for (size_t shot = 0; shot < NSHOTS; ++shot) {
        host_send_ctrl_op(RUN_QKERNEL, &qk_par);
        host_wait_ctrl_op();
        if (qk_par.creg != NULL) qk_par.creg += NMEASURE;
      }
    }
  }

  return status;
}
