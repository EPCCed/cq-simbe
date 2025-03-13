#include <stdlib.h>
#include "host_ops.h"
#include "src/host-device/comms.h"

int alloc_qureg(qubit * qrp, size_t N) {
  if (qrp == NULL) return -2;

  device_alloc_params alloc_params = {
    .NQUBITS = N,
    .qregistry_idx = 0,
    .status = -1
  };

  host_send_ctrl_op(ALLOC, &alloc_params);
  host_wait_ctrl_op();

  if (alloc_params.status == 0) {
    qrp = (qubit *) malloc(sizeof(qubit) * N);
    for (size_t i = 0; i < N; ++i) {
      qrp[i].registry_index = alloc_params.qregistry_idx;
      qrp[i].offset = i;
    }
  } else {
    // Something went wrong!
  }

  return alloc_params.status;
}

int sm_qrun(qkern kernel, qubit * qrp, const size_t NQUBITS, 
  cstate * const crp, const size_t NMEASURE, const size_t NSHOTS) {
    return 0;
}

int free_qureg(qubit * qrp) {
  if (qrp == NULL) return -2;

  device_alloc_params dealloc_params = {
    .NQUBITS = 0,
    .qregistry_idx = qrp[0].registry_index,
    .status = -1
  };

  host_send_ctrl_op(DEALLOC, &dealloc_params);
  host_wait_ctrl_op();

  return dealloc_params.status;
}