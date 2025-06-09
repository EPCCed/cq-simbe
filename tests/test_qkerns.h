#ifndef CQ_TEST_QKERNS_H
#define CQ_TEST_QKERNS_H

#include "datatypes.h"

cq_status qft_circuit(const size_t NQUBITS, qubit * qr);
cq_status zero_init_full_qft(const size_t NQUBITS, qubit * qr, 
  cstate * cr, qkern_map * reg);
cq_status plus_init_full_qft(const size_t NQUBITS, qubit * qr,
  cstate * cr, qkern_map * reg);
cq_status all_site_hadamard(const size_t NQUBITS, qubit * qr,
  cstate * cr, qkern_map * reg);
cq_status only_measure_first_site(const size_t NQUBITS, qubit * qr,
  cstate * cr, qkern_map * reg);
cq_status no_measure_qkern(const size_t NQUBITS, qubit * qr, cstate * cr,
  qkern_map * reg);
cq_status unregistered_kernel(const size_t NQUBITS, qubit * qr,
  cstate * cr, qkern_map * reg);
cq_status overly_long_qkern_name(const size_t NQUBITS, qubit * qr,
  cstate * cr, qkern_map * reg);

#endif