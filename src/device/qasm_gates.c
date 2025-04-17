#include "datatypes.h"
#include "resources.h"
#include "qasm_gates.h"
#include "quest/include/operations.h"

cq_status unitary(qubit * qh, const double THETA, const double PHI,
const double LAMBDA) {
  cq_status status = CQ_ERROR;
  return status;
}

cq_status gphase(qubit * qh, const double THETA) {
  cq_status status = CQ_ERROR;
  return status;
}

cq_status phase(qubit * qh, const double THETA) {
  cq_status status = CQ_ERROR;
  return status;
}

cq_status paulix(qubit * qh) {
  cq_status status = CQ_ERROR;
  return status;
}

cq_status pauliy(qubit * qh) {
  cq_status status = CQ_ERROR;
  return status;
}

cq_status pauliz(qubit * qh) {
  cq_status status = CQ_ERROR;
  return status;
}

cq_status hadamard(qubit * qh) {
  cq_status status = CQ_ERROR;
  
  if (qh != NULL) {
    applyHadamard(qregistry.registers[qh->registry_index], qh->offset);
    status = CQ_SUCCESS;
  }
  
  return status;
}

cq_status sqrtz(qubit * qh) {
  cq_status status = CQ_ERROR;
  return status;
}

cq_status sqrtzhc(qubit * qh) {
  cq_status status = CQ_ERROR;
  return status;
}

cq_status sqrts(qubit * qh) {
  cq_status status = CQ_ERROR;
  return status;
}

cq_status sqrtshc(qubit * qh) { 
  cq_status status = CQ_ERROR;
  return status;
}

cq_status sqrtx(qubit * qh) {
  cq_status status = CQ_ERROR;
  return status;
}

cq_status rotx(qubit * qh, const double THETA) {
  cq_status status = CQ_ERROR;
  return status;
}

cq_status roty(qubit * qh, const double THETA) {
  cq_status status = CQ_ERROR;
  return status;
}

cq_status rotz(qubit * qh, const double THETA) {
  cq_status status = CQ_ERROR;
  return status;
}

cq_status cpaulix(qubit * ctrl, qubit * target) {
  cq_status status = CQ_ERROR;
  return status;
}

cq_status cpauliy(qubit * ctrl, qubit * target) {
  cq_status status = CQ_ERROR;
  return status;
}

cq_status cpauliz(qubit * ctrl, qubit * target) {
  cq_status status = CQ_ERROR;
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

cq_status crotx(qubit * ctrl, qubit * target, const double THETA) {
  cq_status status = CQ_ERROR;
  return status;
}

cq_status croty(qubit * ctrl, qubit * target, const double THETA) {
  cq_status status = CQ_ERROR;
  return status;
}

cq_status crotz(qubit * ctrl, qubit * target, const double THETA) {
  cq_status status = CQ_ERROR;
  return status;
}

cq_status chadamard(qubit * ctrl, qubit * target) {
  cq_status status = CQ_ERROR;
  return status;
}

cq_status cunitary(qubit * ctrl, qubit * target, const double THETA, 
const double PHI, const double LAMBDA) {
  cq_status status = CQ_ERROR;
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

cq_status ccpaulix(qubit * ctrl_a, qubit * ctrl_b, qubit * target) {
  cq_status status = CQ_ERROR;
  return status;
}

cq_status cswap(qubit * ctrl, qubit * a, qubit * b) {
  cq_status status = CQ_ERROR;
  return status;
}