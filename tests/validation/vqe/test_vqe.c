#include "cq.h"
#include "unity.h"
#include "test_vqe.h"

#include <stdlib.h>

#ifndef M_PI
#define M_PI 3.141592653589793238462643383
#endif

const size_t NQUBITS = 2;
const size_t NSHOTS = 10;
const size_t NMEASURE = NQUBITS;
qubit * qr = NULL;
cstate * cr = NULL;

char paulis[2] = {'Z', 'Z'};
double params[2] = {M_PI, M_PI};

static void init_hf_state(qubit * qr, int num_spin_orbitals) {
  for (ptrdiff_t i = 0; i < num_spin_orbitals; ++i) {
    paulix(&qr[i]);
  }
}

static cq_status ansatz(const size_t NQUBITS, qubit * qr, cstate * cr, qkern_map * reg) {
  CQ_REGISTER_KERNEL(reg)

  HANDLE_CQ_ERROR(set_qureg(qr, 0, NQUBITS));
  // HF init state. For H2 we set NQUBITS/2 to 1
  //init_hf_state(qr, NQUBITS / 2);

  const int NLAYERS = 1;
  // preparing RYCZ ansatz
  for (ptrdiff_t i = 0; i < NLAYERS; ++i) {
    for (ptrdiff_t j = 0; j < NQUBITS; ++j) {
      ptrdiff_t param_idx = j + i * (NQUBITS * 2);
      roty(&qr[j], params[param_idx]);

      if (j < NQUBITS - 1) {
        int control = j;
	int target = j + 1;
	cpauliz(&qr[control], &qr[target]);
      }

      // Changing basis to get expectation value
      if (i == NLAYERS - 1) {
	char pauli = paulis[j];
        if (pauli == 'X') {
          hadamard(&qr[j]);
          measure_qubit(&qr[j], &cr[j]);
        
        } else if (pauli == 'Y') {
          rotx(&qr[j], M_PI / 2.0);
          measure_qubit(&qr[j], &cr[j]);
        
        } else if (pauli == 'Z') {
          measure_qubit(&qr[j], &cr[j]);
        
        } else {
          // On I -- do nothing
        }
      }
    }
  }
  return CQ_SUCCESS;
}


void setUp(void) {
  alloc_qureg(&qr, NQUBITS);
  cr = (cstate *) malloc(NMEASURE * NSHOTS * sizeof(cstate));
  return;
}

void tearDown(void) {
  free_qureg(&qr);
  free(cr);
  return;
}

void test_ansatz(void) {
  cstate expected[NMEASURE * NSHOTS];
  init_creg(NMEASURE * NSHOTS, 1, expected);
  init_creg(NMEASURE * NSHOTS, -1, cr);

  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS,
    register_qkern(ansatz));

  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS,
    sm_qrun(ansatz, qr, NQUBITS, cr, NMEASURE, NSHOTS));

  TEST_ASSERT_INT16_ARRAY_WITHIN(0, expected, cr, NMEASURE * NSHOTS);
  init_creg(NMEASURE * NSHOTS, -1, cr);

  cq_exec eh;
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS,
    am_qrun(ansatz, qr, NQUBITS, cr, NMEASURE, NSHOTS, &eh));

  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, wait_qrun(&eh));
  TEST_ASSERT_INT16_ARRAY_WITHIN(0, expected, cr, NMEASURE * NSHOTS);
}



