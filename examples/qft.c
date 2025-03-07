#include <stdlib.h>
#include <math.h>
#include "cq.h"

#ifndef M_PI
#define M_PI 3.141592653589793238462643383
#endif

void qft(const size_t NQUBITS, qubit * qr, cstate * cr) {
  set_qureg(qr, 0, NQUBITS);

  for (size_t i = 0; i < NQUBITS; ++i) {
    hadamard(&qr[i]); 
    for (size_t j = i+1; j < NQUBITS; ++j) {
      double angle = (2 * M_PI) / pow(2, j+1);
      cphase(&qr[j], &qr[i], angle);
    }
  }

  for (size_t i = 0; i < NQUBITS / 2; ++i) {
    size_t j = NQUBITS - (i+1);
    swap(&qr[i], &qr[j]);
  } 
  
  measure_qureg(qr, NQUBITS, cr);
  
  return;
}

int main (void)
{
  const size_t NQUBITS = 10;
  const size_t NSHOTS = 10;
  const size_t NMEASURE = NQUBITS;

  cq_init(1);

  qubit * qr;
  alloc_qureg(qr, NQUBITS);

  cstate * cr;
  cr = malloc(NMEASURE * NSHOTS * sizeof(cstate));

  register_qkern(qft);

  sm_qrun(qft, qr, NQUBITS, cr, NMEASURE, NSHOTS);

  free_qureg(qr);
  free(cr);

  cq_finalise(1);

  return 0;
}