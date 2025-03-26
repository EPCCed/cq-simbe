#include <stdlib.h>
#include "datatypes.h"
#include "host_ops.h"
#include "kernels.h"
#include "src/host-device/comms.h"

cq_status alloc_qureg(qubit ** qrp, size_t N) {
  device_alloc_params alloc_params = {
    .NQUBITS = N,
    .qregistry_idx = 0,
    .status = CQ_ERROR
  };

  host_send_ctrl_op(ALLOC, &alloc_params);
  host_wait_ctrl_op();

  if (alloc_params.status == CQ_SUCCESS) {
    *qrp = (qubit *) malloc(sizeof(qubit) * N);
    for (size_t i = 0; i < N; ++i) {
      (*qrp)[i].registry_index = alloc_params.qregistry_idx;
      (*qrp)[i].offset = i;
    }
  } else {
    // Something went wrong!
  }

  return alloc_params.status;
}

cq_status sm_qrun(qkern kernel, qubit * qrp, const size_t NQUBITS, 
cstate * const crp, const size_t NMEASURE, const size_t NSHOTS) {
  int status = 0;
  char const * fname = NULL;

  status = find_qkern_name(kernel, &fname);    

  if (!status) {
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
      qk_par.creg += NMEASURE;
    }
  }

  return status;
}

cq_status free_qureg(qubit * qrp) {
  if (qrp == NULL) return CQ_WARNING;

  device_alloc_params dealloc_params = {
    .NQUBITS = 0,
    .qregistry_idx = qrp[0].registry_index,
    .status = CQ_ERROR
  };

  host_send_ctrl_op(DEALLOC, &dealloc_params);
  host_wait_ctrl_op();

  free(qrp);

  return dealloc_params.status;
}