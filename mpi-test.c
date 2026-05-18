#include <stdio.h>
#include "cq.h"

cq_status test_qkern(const size_t NQUBITS,
                     qubit* qr,
                     const size_t NMEASURE,
                     cstate* cr,
                     qkern_map* reg) {
  CQ_REGISTER_KERNEL(reg)
  set_qureg(qr, 0, NQUBITS);
  for (size_t i = 0; i < NQUBITS; ++i) {
    hadamard(&qr[i]);
  }
  measure_qureg(qr, NQUBITS, cr);
  return CQ_SUCCESS;
}

int main() {
  const size_t NQUBITS = 10;
  const size_t NSHOTS = 10;
  const size_t NMEASURE = NQUBITS;

  cq_init(1);
  qubit* qr;
  alloc_qureg(&qr, NQUBITS);
  //  if (get_rank() == 0) {
  //    printf(
  //        "[MAIN]: created qreg: with NQUBITS: %zu, qregistry_idx: %zu,
  //        offset: "
  //        "%zu\n",
  //        qr->N, qr->registry_index, qr->offset);
  //    printf("\n\nresult of free qureg: %d\n\n", free_qureg(&qr));
  //  }

  cstate cr[NMEASURE * NSHOTS];
  init_creg(NMEASURE * NSHOTS, -1, cr);
  register_qkern(test_qkern);

  cq_exec eh;
  // a_qrun(test_qkern, qr, NQUBITS, cr, NMEASURE, &eh);
  // wait_qrun(&eh);
  // s_qrun(test_qkern, qr, NQUBITS, cr, NMEASURE);
  sm_qrun(test_qkern, qr, NQUBITS, cr, NMEASURE, NSHOTS);
  // am_qrun(test_qkern, qr, NQUBITS, cr, NMEASURE, NSHOTS, &eh);
  //  halt_qrun(&eh);
  // wait_qrun(&eh);

  free_qureg(&qr);
  cq_finalise(1);
  return 0;
}
