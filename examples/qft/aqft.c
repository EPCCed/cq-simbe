#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "qft.h"
#include "cq.h"

int main (void)
{
  const size_t NQUBITS = 10;
  const size_t NSHOTS = 10;
  const size_t NMEASURE = NQUBITS;

  cq_exec eh_zero, eh_plus;

  cq_init(0);

  // We will reuse the quantum buffer as the quantum
  // kernels cannot run simultaneously (for now...)
  qubit * qr = NULL;
  alloc_qureg(&qr, NQUBITS);

  cstate cr_zero[NMEASURE * NSHOTS];
  cstate cr_plus[NMEASURE * NSHOTS];

  init_creg(NMEASURE * NSHOTS, -1, cr_zero);
  init_creg(NMEASURE * NSHOTS, -1, cr_plus);

  register_qkern(zero_init_full_qft);
  register_qkern(plus_init_full_qft);

  printf("Offloading both QFT circuits to the quantum device.\n");
  am_qrun(zero_init_full_qft, qr, NQUBITS, cr_zero, NMEASURE, NSHOTS, &eh_zero);
  am_qrun(plus_init_full_qft, qr, NQUBITS, cr_plus, NMEASURE, NSHOTS, &eh_plus);

  printf("Hello from the host, pretend I'm doing something useful!\n");
  sleep(2);
  printf("Hello again, I'm done being 'useful' and will now wait for "); 
  printf("the quantum device to return!\n");

  wait_qrun(&eh_zero);
  printf("Results from zero-initialised QFT:\n");
  report_results(cr_zero, NMEASURE, NSHOTS);

  wait_qrun(&eh_plus); 
  printf("Results from plus-initialised QFT:\n");
  report_results(cr_plus, NMEASURE, NSHOTS);

  free_qureg(&qr);

  cq_finalise(0);

  return 0;
}