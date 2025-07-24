#include <stdlib.h>
#include <stdio.h>
#include "cq.h"
#include "vqe.h"

hamiltonian h2_hamil = {
	.paulis = { 
	        'I', 'I',
		'I', 'Z',
		'Z', 'I',
		'Z', 'Z',	
		'X', 'X' },
	.coeffs = {
        	-1.052373245772859,
		0.39793742484318045,
		-0.39793742484318045,
		-0.01128010425623538,
		0.18093119978423156},
	.term_start_idx = 0
};

int main (void) {
  const double TRUE_ENERGY = -1.85728;
  const size_t NQUBITS = 2;
  const size_t NSHOTS = 2024;
  const size_t NMEASURE = NQUBITS;

  cq_init(0);

  qubit * qr = NULL;
  alloc_qureg(&qr, NQUBITS);

  cstate * cr;
  cr = malloc(NMEASURE * NSHOTS * sizeof(cstate));
  init_creg(NMEASURE * NSHOTS, -1, cr);

  register_qkern(ansatz);

  double energy = vqe_optimize(qr, cr, NQUBITS, NMEASURE, NSHOTS);
  printf("FCI Energy: %f\n"
         "VQE Energy: %f\n"
         "VQE Error: %f\n",
	 TRUE_ENERGY,
	 energy,
	 TRUE_ENERGY - energy);
 
  free_qureg(&qr);
  free(cr);

  cq_finalise(0);

  return 0;
}
