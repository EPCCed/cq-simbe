#include <stdio.h>
#include <stdlib.h>
#include "cq.h"
#include "qft.h"

int main(void) {
  const size_t NQUBITS = 10, NSHOTS = 10, NMEASURE = NQUBITS;
  int failed = 0;
  if (cq_init(0) != CQ_SUCCESS) return EXIT_FAILURE;
  if (register_qkern(zero_init_full_qft) != CQ_SUCCESS ||
      register_qkern(plus_init_full_qft) != CQ_SUCCESS) {
    cq_finalise(0);
    return EXIT_FAILURE;
  }
  CQ_PROG_BEGIN()
  qubit *qr = NULL;
  cstate *cr = NULL;
  if (alloc_qureg(&qr, NQUBITS) != CQ_SUCCESS ||
      !(cr = malloc(NMEASURE * NSHOTS * sizeof(cstate)))) {
    failed = 1;
  } else {
    init_creg(NMEASURE * NSHOTS, -1, cr);
    printf("Running first QFT circuit on quantum device.\n");
    if (sm_qrun(zero_init_full_qft, qr, NQUBITS, cr, NMEASURE, NSHOTS) != CQ_SUCCESS) failed = 1;
    else report_results(cr, NMEASURE, NSHOTS);
    printf("Running second QFT circuit on quantum device.\n");
    if (sm_qrun(plus_init_full_qft, qr, NQUBITS, cr, NMEASURE, NSHOTS) != CQ_SUCCESS) failed = 1;
    else report_results(cr, NMEASURE, NSHOTS);
  }
  if (qr && free_qureg(&qr) != CQ_SUCCESS) failed = 1;
  free(cr);
  CQ_PROG_END()
  if (cq_finalise(0) != CQ_SUCCESS) failed = 1;
  return failed ? EXIT_FAILURE : EXIT_SUCCESS;
}
