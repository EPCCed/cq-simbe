#include "qft.h"
#include "cq.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

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
  cstate cr_zero[NMEASURE * NSHOTS], cr_plus[NMEASURE * NSHOTS];
  cq_exec eh_zero, eh_plus;
  if (alloc_qureg(&qr, NQUBITS) != CQ_SUCCESS) {
    failed = 1;
  } else {
    init_creg(NMEASURE * NSHOTS, -1, cr_zero);
    init_creg(NMEASURE * NSHOTS, -1, cr_plus);
    printf("Offloading both QFT circuits to the quantum device.\n");
    int zero_started = am_qrun(zero_init_full_qft, qr, NQUBITS, cr_zero,
                               NMEASURE, NSHOTS, &eh_zero) == CQ_SUCCESS;
    int plus_started = am_qrun(plus_init_full_qft, qr, NQUBITS, cr_plus,
                               NMEASURE, NSHOTS, &eh_plus) == CQ_SUCCESS;
    if (!zero_started || !plus_started) failed = 1;
    printf("Hello from the host, pretend I'm doing something useful!\n");
    sleep(2);
    if (zero_started && (wait_qrun(&eh_zero) != CQ_SUCCESS || eh_zero.status != CQ_SUCCESS)) failed = 1;
    if (plus_started && (wait_qrun(&eh_plus) != CQ_SUCCESS || eh_plus.status != CQ_SUCCESS)) failed = 1;
    if (!failed) {
      printf("Results from zero-initialised QFT:\n");
      report_results(cr_zero, NMEASURE, NSHOTS);
      printf("Results from plus-initialised QFT:\n");
      report_results(cr_plus, NMEASURE, NSHOTS);
    }
    if (free_qureg(&qr) != CQ_SUCCESS) failed = 1;
  }
  CQ_PROG_END()
  if (cq_finalise(0) != CQ_SUCCESS) failed = 1;
  return failed ? EXIT_FAILURE : EXIT_SUCCESS;
}
