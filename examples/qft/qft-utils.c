#include <stddef.h>
#include <stdio.h>
#include <math.h>
#include "qft.h"
#include "cq.h"

#ifndef M_PI
#define M_PI 3.141592653589793238462643383
#endif

void report_results(cstate const * const CR, const size_t NMEASURE,
const size_t NSHOTS) {
  printf("Reporting measurement outcomes:\n");
  for (size_t shot = 0; shot < NSHOTS; ++shot) {
    printf("Shot [%lu]: ", shot);
    for (size_t i = shot * NMEASURE; i < (shot + 1) * NMEASURE; ++i) {
      printf("%d ", CR[i]);
    }
    printf("\n");
  }

  return;
}

static cq_status checked_qft_circuit(const size_t NQUBITS, qubit * qr) {
  // Run QFT
  for (size_t i = 0; i < NQUBITS; ++i) {
    HANDLE_CQ_ERROR(hadamard(&qr[i]));
    for (size_t j = i+1; j < NQUBITS; ++j) {
      double angle = M_PI / pow(2, j);
      HANDLE_CQ_ERROR(cphase(&qr[j], &qr[i], angle));
    }
  }

  for (size_t i = 0; i < NQUBITS / 2; ++i) {
    size_t j = NQUBITS - (i+1);
    HANDLE_CQ_ERROR(swap(&qr[i], &qr[j]));
  }

  return CQ_SUCCESS;
}

void full_qft_circuit(const size_t NQUBITS, qubit *qr) {
  (void)checked_qft_circuit(NQUBITS, qr);
}

cq_status zero_init_full_qft(
const size_t NQUBITS, qubit * qr, const size_t NMEASURE, cstate * cr, qkern_map * reg) {
  CQ_REGISTER_KERNEL(reg)

  // Prepare state
  if (NMEASURE < NQUBITS) return CQ_ERROR;
  HANDLE_CQ_ERROR(set_qureg(qr, 0, NQUBITS));

  // Run QFT
  HANDLE_CQ_ERROR(checked_qft_circuit(NQUBITS, qr));

  // Measure
  HANDLE_CQ_ERROR(measure_qureg(qr, NQUBITS, cr));

  return CQ_SUCCESS;
}

cq_status plus_init_full_qft(
const size_t NQUBITS, qubit * qr, const size_t NMEASURE, cstate * cr, qkern_map * reg) {
  CQ_REGISTER_KERNEL(reg);

  // Prepare state
  if (NMEASURE < NQUBITS) return CQ_ERROR;
  HANDLE_CQ_ERROR(set_qureg(qr, 0, NQUBITS));
  for (size_t i = 0; i < NQUBITS; ++i) {
    HANDLE_CQ_ERROR(hadamard(&qr[i]));
  }

  // Run QFT
  HANDLE_CQ_ERROR(checked_qft_circuit(NQUBITS, qr));

  // Measure
  HANDLE_CQ_ERROR(measure_qureg(qr, NQUBITS, cr));

  return CQ_SUCCESS;
}
