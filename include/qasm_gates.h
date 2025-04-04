#ifndef CQ_QASM_GATES_H
#define CQ_QASM_GATES_H

#include "datatypes.h"

cq_status unitary(qubit * qh, const double THETA, const double PHI, 
  const double LAMBDA);

cq_status gphase(qubit * qh, const double THETA);

cq_status phase(qubit * qh, const double THETA);

cq_status paulix(qubit * qh);

cq_status pauliy(qubit * qh);

cq_status pauliz(qubit * qh);

cq_status hadamard(qubit * qh);

cq_status sqrtz(qubit * qh);

cq_status sqrtzhc(qubit * qh);

cq_status sqrts(qubit * qh);

cq_status sqrtshc(qubit * qh);

cq_status sqrtx(qubit * qh);

cq_status rotx(qubit * qh, const double THETA);

cq_status roty(qubit * qh, const double THETA);

cq_status rotz(qubit * qh, const double THETA);

cq_status cpaulix(qubit * ctrl, qubit * target);

cq_status cpauliy(qubit * ctrl, qubit * target);

cq_status cpauliz(qubit * ctrl, qubit * target);

cq_status cphase(qubit * ctrl, qubit * target, const double THETA);

cq_status crotx(qubit * ctrl, qubit *  target, const double THETA);

cq_status croty(qubit * ctrl, qubit *  target, const double THETA);

cq_status crotz(qubit * ctrl, qubit *  target, const double THETA);

cq_status chadamard(qubit * ctrl, qubit * target);

cq_status cunitary(qubit * ctrl, qubit * target, const double THETA, 
  const double PHI, const double LAMBDA);

cq_status swap(qubit * a, qubit * b);

cq_status ccpaulix(qubit * ctrl_a, qubit * ctrl_b, qubit * target);

cq_status cswap(qubit * ctrl, qubit * a, qubit * b);

#endif