#include <stdlib.h>
#include <math.h>
#include "unity.h"
#include "cq.h"
#include "tests/test_qkerns.h"

#ifndef M_PI
#define M_PI 3.141592653589793238462643383
#endif

const size_t NQUBITS = 10;
const size_t NSHOTS = 10;
const size_t NMEASURE = NQUBITS;
qubit * qr = NULL;
cstate * cr = NULL;

// runs before every test
void setUp(void) {
  alloc_qureg(&qr, NQUBITS);
  cr = (cstate *) malloc(NMEASURE * NSHOTS * sizeof(cstate));
  return;
}

// runs after every test
void tearDown(void) {
  free_qureg(&qr);
  free(cr);
  return;
}

void test_zero_init_qft(void) {
  cstate expected[NMEASURE * NSHOTS];
  
  init_creg(NMEASURE * NSHOTS, 1, expected);
  init_creg(NMEASURE * NSHOTS, -1, cr);

  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, register_qkern(zero_init_full_qft));
  
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, 
    sm_qrun(zero_init_full_qft, qr, NQUBITS, cr, NMEASURE, NSHOTS));

  // tests that all measurements are within range 0-2 (so not still -1)
  TEST_ASSERT_INT16_ARRAY_WITHIN(1, expected, cr, NMEASURE * NSHOTS);

  init_creg(NMEASURE * NSHOTS, -1, cr);

  cq_exec eh;
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, 
    am_qrun(zero_init_full_qft, qr, NQUBITS, cr, NMEASURE, NSHOTS, &eh));

  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, wait_qrun(&eh));

  // tests that all measurements are within range 0-2 (so not still -1)
  TEST_ASSERT_INT16_ARRAY_WITHIN(1, expected, cr, NMEASURE * NSHOTS);

  return;
}

void test_plus_init_qft(void) {
  init_creg(NMEASURE * NSHOTS, -1, cr);

  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, register_qkern(plus_init_full_qft));

  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, 
    sm_qrun(plus_init_full_qft, qr, NQUBITS, cr, NMEASURE, NSHOTS));

  TEST_ASSERT_EACH_EQUAL_INT16(0, cr, NMEASURE * NSHOTS);

  init_creg(NMEASURE * NSHOTS, -1, cr);
 
  cq_exec eh;
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, 
    am_qrun(
      plus_init_full_qft, qr, NQUBITS, cr, NMEASURE, NSHOTS, &eh
    )
  );

  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, wait_qrun(&eh));

  TEST_ASSERT_EACH_EQUAL_INT16(0, cr, NMEASURE * NSHOTS);
  
  return;
}