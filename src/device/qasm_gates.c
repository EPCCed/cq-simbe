#include "datatypes.h"
#include "resources.h"
#include "qasm_gates.h"
#include "quest/include/operations.h"

#ifndef NDEBUG
#include <stdio.h>
#endif

int hadamard(qubit * qh) {
  applyHadamard(qregistry.registers[qh->registry_index], qh->offset);
  return 0;
}

int cphase(qubit * ctrl, qubit * target, const double THETA) {
  Qureg qureg = qregistry.registers[ctrl->registry_index];
  applyTwoQubitPhaseShift(qureg, ctrl->offset, target->offset, THETA);
  return 0;
}

int swap(qubit * a, qubit * b) {
  Qureg qureg = qregistry.registers[a->registry_index];
  applySwap(qureg, a->offset, b->offset);
  return 0;
}