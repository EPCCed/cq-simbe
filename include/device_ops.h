#ifndef CQ_DEVICE_OPS_H
#define CQ_DEVICE_OPS_H

#include <stddef.h>
#include "datatypes.h"

// Resource management

int set_qubit(qubit qh, cstate cs);

int set_qureg(qubit * qrp, const unsigned long long STATE_IDX, const size_t N);

int set_qureg_cstate(qubit * qrp, cstate const * const CRP, const size_t N);

// Control

int qabort(const unsigned int STATUS);

// Measurements

int dmeasure_qubit(qubit * qbp, cstate * csp);

int dmeasure_qureg(qubit * qr, const size_t NQUBITS, cstate * cr);

int dmeasure(qubit * qr, const size_t NQUBITS, size_t const * const TARGETS, 
  const size_t NTARGETS, cstate * cr);

int measure_qubit(qubit * qbp, cstate * csp);

int measure_qureg(qubit * qr, const size_t NQUBITS, cstate * cr);

int measure(qubit * qr, const size_t NQUBITS, size_t const * const TARGETS, 
  const size_t NTARGETS, cstate * cr);

#endif