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

void full_qft_circuit(const size_t NQUBITS, qubit * qr) {
  // Run QFT
  for (size_t i = 0; i < NQUBITS; ++i) {
    hadamard(&qr[i]); 
    for (size_t j = i+1; j < NQUBITS; ++j) {
      double angle = M_PI / pow(2, j);
      cphase(&qr[j], &qr[i], angle);
    }
  }

  for (size_t i = 0; i < NQUBITS / 2; ++i) {
    size_t j = NQUBITS - (i+1);
    swap(&qr[i], &qr[j]);
  } 

  return;
}

cq_status zero_init_full_qft(
const size_t NQUBITS, qubit * qr, cstate * cr, qkern_map * reg) {
  CQ_REGISTER_KERNEL(reg)

  // Prepare state
  set_qureg(qr, 0, NQUBITS);

  // Run QFT
  full_qft_circuit(NQUBITS, qr);

  // Measure
  measure_qureg(qr, NQUBITS, cr);
  
  return CQ_SUCCESS;
}

cq_status plus_init_full_qft(
const size_t NQUBITS, qubit * qr, cstate * cr, qkern_map * reg) {
  CQ_REGISTER_KERNEL(reg);

  // Prepare state
  set_qureg(qr, 0, NQUBITS);
  for (size_t i = 0; i < NQUBITS; ++i) {
    hadamard(&qr[i]);
  }

  // Run QFT
  full_qft_circuit(NQUBITS, qr);

  // Measure
  measure_qureg(qr, NQUBITS, cr);

  return CQ_SUCCESS;
}