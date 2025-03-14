#include "device_ops.h"
#include "resources.h"
#include "device_utils.h"
#include "quest/include/initialisations.h"
#include "quest/include/operations.h"

#ifndef NDEBUG
#include <stdio.h>
#endif

int set_qureg(qubit * qrp, const unsigned long long STATE_IDX, const size_t N) {
  initClassicalState(qregistry.registers[qrp->registry_index], STATE_IDX);
  return 0;
}

int measure_qureg(qubit * qr, const size_t NQUBITS, cstate * cr) {
  int targets[NQUBITS];
  for (int i = 0; i < NQUBITS; ++i) {
    targets[i] = i;
  }

#ifndef NDEBUG
  reportQureg(qregistry.registers[qr[0].registry_index]);
  qreal prob;
  qindex outcome = applyMultiQubitMeasurementAndGetProb(
    qregistry.registers[qr[0].registry_index], targets, NQUBITS, &prob
  );
  printf("Outcome %lld with probability %g.\n", outcome, prob);
#else
  qindex outcome = applyMultiQubitMeasurement(
    qregistry.registers[qr[0].registry_index], targets, NQUBITS
  );
#endif

  qindex_to_cstate(outcome, cr, NQUBITS);
  return 0;
}