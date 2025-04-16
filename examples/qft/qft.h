#ifndef CQ_QFT_EXAMPLE_H
#define CQ_QFT_EXAMPLE_H

#include "cq.h"

void report_results(cstate const * const CR, const size_t NMEASURE, 
  const size_t NSHOTS);
void full_qft_circuit(const size_t NQUBITS, qubit * qr);
void zero_init_full_qft(const size_t NQUBITS, qubit * qr, cstate * cr,
  qkern_map * reg);
void plus_init_full_qft(const size_t NQUBITS, qubit * qr, cstate * cr, 
  qkern_map * reg);

#endif