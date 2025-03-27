#ifndef CQ_TEST_QKERNS_H
#define CQ_TEST_QKERNS_H

#include "datatypes.h"

void qft_circuit(const size_t NQUBITS, qubit * qr);
void zero_init_full_qft(const size_t NQUBITS, qubit * qr, 
  cstate * cr, qkern_map * reg);
void equal_superposition_full_qft(const size_t NQUBITS, qubit * qr,
  cstate * cr, qkern_map * reg);
void all_site_hadamard(const size_t NQUBITS, qubit * qr,
  cstate * cr, qkern_map * reg);
void unregistered_kernel(const size_t NQUBITS, qubit * qr,
  cstate * cr, qkern_map * reg);

#endif