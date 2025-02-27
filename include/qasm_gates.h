#ifndef CQ_QASM_GATES_H
#define CQ_QASM_GATES_H

#include "datatypes.h"

int unitary(qubit * qh, const double THETA, const double PHI, 
  const double LAMBDA);

int gphase(qubit * qh, const double THETA);

int phase(qubit * qh, const double THETA);

int paulix(qubit * qh);

int pauliy(qubit * qh);

int pauliz(qubit * qh);

int hadamard(qubit * qh);

int sqrtz(qubit * qh);

int sqrtzhc(qubit * qh);

int sqrts(qubit * qh);

int sqrtshc(qubit * qh);

int sqrtx(qubit * qh);

int rotx(qubit * qh, const double THETA);

int roty(qubit * qh, const double THETA);

int rotz(qubit * qh, const double THETA);

int cpaulix(qubit * ctrl, qubit * target);

int cpauliy(qubit * ctrl, qubit * target);

int cpauliz(qubit * ctrl, qubit * target);

int cphase(qubit * ctrl, qubit * target, const double THETA);

int crotx(qubit * ctrl, qubit *  target, const double THETA);

int croty(qubit * ctrl, qubit *  target, const double THETA);

int crotz(qubit * ctrl, qubit *  target, const double THETA);

int chadamard(qubit * ctrl, qubit * target);

int cunitary(qubit * ctrl, qubit * target, const double THETA, 
  const double PHI, const double LAMBDA);

int swap(qubit * a, qubit * b);

int ccpaulix(qubit * ctrl_a, qubit * ctrl_b, qubit * target);

int cswap(qubit * ctrl, qubit * a, qubit * b);

#endif