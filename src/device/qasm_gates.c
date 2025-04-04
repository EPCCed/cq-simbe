#include "datatypes.h"
#include "resources.h"
#include "qasm_gates.h"
#include "quest/include/operations.h"

cq_status hadamard(qubit * qh) {
  cq_status status = CQ_ERROR;
  
  if (qh != NULL) {
    applyHadamard(qregistry.registers[qh->registry_index], qh->offset);
    status = CQ_SUCCESS;
  }
  
  return status;
}

cq_status cphase(qubit * ctrl, qubit * target, const double THETA) {
  cq_status status = CQ_ERROR;

  if (ctrl != target && ctrl != NULL && target != NULL) {
    Qureg qureg = qregistry.registers[ctrl->registry_index];
    applyTwoQubitPhaseShift(qureg, ctrl->offset, target->offset, THETA);
    status = CQ_SUCCESS;
  }

  return status;
}

cq_status swap(qubit * a, qubit * b) {
  cq_status status = CQ_ERROR;

  if (a != b && a != NULL && b != NULL) {
    Qureg qureg = qregistry.registers[a->registry_index];
    applySwap(qureg, a->offset, b->offset);
    status = CQ_SUCCESS;
  }
  
  return status;
}