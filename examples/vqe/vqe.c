#include "vqe.h"
#include "cq.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

hamiltonian h2_hamil
    = { .paulis = { 'I', 'I', 'I', 'Z', 'Z', 'I', 'Z', 'Z', 'X', 'X' },
        .coeffs
        = { -1.052373245772859, 0.39793742484318045, -0.39793742484318045,
            -0.01128010425623538, 0.18093119978423156 } };

int main(void) {
  const double TRUE_ENERGY = -1.85728;
  const size_t NQUBITS = 2;
  const size_t NSHOTS = 2000;
  const size_t NMEASURE = NQUBITS;
  int failed = 0;

  if (cq_init(0) != CQ_SUCCESS) return EXIT_FAILURE;
  if (register_pqkern(ansatz) != CQ_SUCCESS) {
    cq_finalise(0);
    return EXIT_FAILURE;
  }

  CQ_PROG_BEGIN();

  qubit * qr = NULL;
  cstate * cr = NULL;
  if (alloc_qureg(&qr, NQUBITS) != CQ_SUCCESS ||
      !(cr = malloc(NMEASURE * NSHOTS * sizeof(cstate)))) {
    fprintf(stderr, "VQE allocation failed.\n");
    failed = 1;
  } else {
    init_creg(NMEASURE * NSHOTS, -1, cr);
    double energy = vqe_optimize(qr, cr, NQUBITS, NMEASURE, NSHOTS);
    if (!isfinite(energy)) {
      fprintf(stderr, "VQE optimization failed.\n");
      failed = 1;
    } else printf(
      "FCI Energy: %f\n"
      "VQE Energy: %f\n"
      "VQE Error: %f\n",
      TRUE_ENERGY, energy, TRUE_ENERGY - energy);
  }

  if (qr && free_qureg(&qr) != CQ_SUCCESS) failed = 1;
  free(cr);

  CQ_PROG_END();

  if (cq_finalise(0) != CQ_SUCCESS) failed = 1;

  return failed ? EXIT_FAILURE : EXIT_SUCCESS;
}
