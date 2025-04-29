#ifndef CQ_DEVICE_OPS_H
#define CQ_DEVICE_OPS_H

#include <stddef.h>
#include "datatypes.h"

// Resource management

cq_status set_qubit(qubit qh, cstate cs);

cq_status set_qureg(qubit * qrp, const unsigned long long STATE_IDX, const size_t N);

cq_status set_qureg_cstate(qubit * qrp, cstate const * const CRP, const size_t N);

// Control

cq_status qabort(const unsigned int STATUS);

// Measurements

cq_status dmeasure_qubit(qubit * qbp, cstate * csp);

cq_status dmeasure_qureg(qubit * qr, const size_t NQUBITS, cstate * cr);

cq_status dmeasure(qubit * qr, const size_t NQUBITS, size_t const * const TARGETS, 
  const size_t NTARGETS, cstate * cr);

cq_status measure_qubit(qubit * qbp, cstate * csp);

cq_status measure_qureg(qubit * qr, const size_t NQUBITS, cstate * cr);

cq_status measure(qubit * qr, const size_t NQUBITS, size_t const * const TARGETS, 
  const size_t NTARGETS, cstate * cr);

#endif