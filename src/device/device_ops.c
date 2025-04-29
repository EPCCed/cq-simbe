#include "device_ops.h"
#include "resources.h"
#include "device_utils.h"
#include "quest/include/initialisations.h"
#include "quest/include/operations.h"

cq_status set_qubit(qubit qh, cstate cs) {
  cq_status status = CQ_ERROR;
  return status;
}

cq_status set_qureg(qubit * qrp, const unsigned long long STATE_IDX, const size_t N) {
  cq_status status = CQ_ERROR;

  // 2^N
  const unsigned long long NBIT_STATES = (1llu << N);
  
  if (qrp != NULL && STATE_IDX < NBIT_STATES && N <= qrp[0].N) {
    initClassicalState(qregistry.registers[qrp->registry_index], STATE_IDX);
    status = CQ_SUCCESS;
  }

  return status;
}

cq_status measure_qureg(qubit * qr, const size_t NQUBITS, cstate * cr) {
  cq_status status = CQ_ERROR;

  if (qr != NULL && cr != NULL && NQUBITS <= qr[0].N) {
    int targets[NQUBITS];
    for (int i = 0; i < NQUBITS; ++i) {
      targets[i] = i;
    }

    qindex outcome = applyMultiQubitMeasurement(
      qregistry.registers[qr[0].registry_index], targets, NQUBITS
    );

    qindex_to_cstate(outcome, cr, NQUBITS);

    status = CQ_SUCCESS;
  }

  return status;
}