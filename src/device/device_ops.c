#include "device_ops.h"
#include "resources.h"
#include "device_utils.h"
#include "quest/include/initialisations.h"
#include "quest/include/operations.h"

cq_status set_qubit(qubit qh, cstate cs) {
  // to avoid normalisation issues we fix the qubit to its
  // "natural" classical state, then flip if needed
  cq_status status = CQ_ERROR;

  if (cs > -1 && cs < 2) {
    Qureg qureg = qregistry.registers[qh.registry_index];
    cstate outcome = applyQubitMeasurement(qureg, qh.offset);
    if (outcome != cs) {
      applyPauliX(qureg, qh.offset);
    }
    status = CQ_SUCCESS;
  }

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

cq_status set_qureg_cstate(qubit * qrp, cstate const * const CRP, const size_t N) {
  cq_status status = CQ_ERROR;

  if (CRP != NULL && qrp != NULL && N <= qrp->N) {
    qindex state;
    status = cstate_to_qindex(CRP, &state, N);
    if (status == CQ_SUCCESS) {
      initClassicalState(qregistry.registers[qrp->registry_index], state);
    }
  }

  return status;
}

cq_status measure_qubit(qubit * qbp, cstate * csp) {
  cq_status status = CQ_ERROR;

  if (qbp != NULL && csp != NULL) {
    *csp = applyQubitMeasurement(qregistry.registers[qbp->registry_index], qbp->offset);
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