#include <stdlib.h>
#include <stdio.h>
#include "qft.h"
#include "cq.h"

int main (void)
{
  const size_t NQUBITS = 10;
  const size_t NSHOTS = 10;
  const size_t NMEASURE = NQUBITS;

  cq_init(0);

  qubit * qr = NULL;
  alloc_qureg(&qr, NQUBITS);

  cstate * cr;
  cr = malloc(NMEASURE * NSHOTS * sizeof(cstate));
  init_creg(NMEASURE * NSHOTS, -1, cr);

  register_qkern(zero_init_full_qft);
  register_qkern(plus_init_full_qft);

  printf("Running first QFT circuit on quantum device.\n");
  sm_qrun(zero_init_full_qft, qr, NQUBITS, cr, NMEASURE, NSHOTS);
  report_results(cr, NMEASURE, NSHOTS);

  printf("Running second QFT circuit on quantum device.\n");
  sm_qrun(plus_init_full_qft, qr, NQUBITS, cr, NMEASURE, NSHOTS);
  report_results(cr, NMEASURE, NSHOTS);

  free_qureg(&qr);
  free(cr);

  cq_finalise(0);

  return 0;
}