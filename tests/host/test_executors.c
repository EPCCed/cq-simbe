#include <stdlib.h>
#include "unity.h"
#include "host_ops.h"
#include "utils.h"
#include "test_executors.h"

const size_t NQUBITS = 5;

void setUp(void) {
  return;
}

void tearDown(void) {
  return;
}

void test_first_run(void) {
  const size_t NMEASURE = NQUBITS;
  const size_t NSHOTS = 1;
  const cstate CR_INIT_VAL = -1;

  cstate * cr;
  cr = (cstate*) malloc(NMEASURE*NSHOTS*sizeof(cstate));

  cstate * expected;
  expected = (cstate*) malloc(NMEASURE*NSHOTS*sizeof(cstate));
  init_creg(NMEASURE*NSHOTS, 1, expected);

  qubit * qr = NULL;
  alloc_qureg(&qr, NQUBITS);

  init_creg(NMEASURE*NSHOTS, CR_INIT_VAL, cr);
  TEST_ASSERT_EACH_EQUAL_INT8(CR_INIT_VAL, cr, NMEASURE*NSHOTS);
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, 
    sm_qrun(zero_init_full_qft, qr, NQUBITS, cr, NMEASURE, NSHOTS)
  );
  // tests that all elements with cr are in the range 0-2 i.e. not -1
  TEST_ASSERT_INT8_ARRAY_WITHIN(1, expected, cr, NMEASURE*NSHOTS);

  init_creg(NMEASURE*NSHOTS, CR_INIT_VAL, cr);
  TEST_ASSERT_EQUAL_INT8(CQ_SUCCESS,
    sm_qrun(equal_superposition_full_qft, qr, NQUBITS, cr, NMEASURE, NSHOTS)
  );
  TEST_ASSERT_INT8_ARRAY_WITHIN(1, expected, cr, NMEASURE*NSHOTS);

  init_creg(NMEASURE*NSHOTS, CR_INIT_VAL, cr);
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS,
    sm_qrun(all_site_hadamard, qr, NQUBITS, cr, NMEASURE, NSHOTS)
  );
  TEST_ASSERT_INT8_ARRAY_WITHIN(1, expected, cr, NMEASURE*NSHOTS);

  init_creg(NMEASURE*NSHOTS, CR_INIT_VAL, cr);
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS,
    // we use NMEASURE=1 here
    sm_qrun(only_measure_first_site, qr, NQUBITS, cr, 1, NSHOTS)
  );
  TEST_ASSERT_INT8_ARRAY_WITHIN(1, expected, cr, NSHOTS);
  TEST_ASSERT_EACH_EQUAL_INT8(CR_INIT_VAL, cr+NSHOTS, (NMEASURE-1)*NSHOTS);

  // no measurements here!
  init_creg(NSHOTS*NMEASURE, CR_INIT_VAL, cr);
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS,
    sm_qrun(no_measure_qkern, qr, NQUBITS, cr, NMEASURE, NSHOTS)
  );
  TEST_ASSERT_EACH_EQUAL_INT8(CR_INIT_VAL, cr, NSHOTS*NMEASURE);

  free_qureg(&qr);
  free(cr);

  return;
}

void test_nmeasure(void) {
  const size_t NSHOTS = 2;
  const cstate CR_INIT_VAL = -1;
  size_t nmeasure;
  cstate * cr, * expected;
  
  qubit * qr = NULL;
  alloc_qureg(&qr, NQUBITS);

  nmeasure = 0;
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS,
    sm_qrun(no_measure_qkern, qr, NQUBITS, NULL, nmeasure, NSHOTS)
  );


  nmeasure = 1;
  cr = (cstate*) malloc(nmeasure*NSHOTS*sizeof(cstate)); 
  expected = (cstate*) malloc(nmeasure*NSHOTS*sizeof(cstate));
  
  init_creg(nmeasure*NSHOTS, CR_INIT_VAL, cr);
  init_creg(nmeasure*NSHOTS, 1, expected);
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS,
    sm_qrun(only_measure_first_site, qr, NQUBITS, cr, nmeasure, NSHOTS)
  );
  TEST_ASSERT_INT8_ARRAY_WITHIN(1, expected, cr, nmeasure*NSHOTS);
  free(cr);
  free(expected);

  nmeasure = NQUBITS;
  cr = (cstate*) malloc(nmeasure*NSHOTS*sizeof(cstate)); 
  expected = (cstate*) malloc(nmeasure*NSHOTS*sizeof(cstate));
  init_creg(nmeasure*NSHOTS, CR_INIT_VAL, cr);
  init_creg(nmeasure*NSHOTS, 1, expected);
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS,
    sm_qrun(all_site_hadamard, qr, NQUBITS, cr, nmeasure, NSHOTS)
  );
  TEST_ASSERT_INT8_ARRAY_WITHIN(1, expected, cr, nmeasure*NSHOTS);
  free(cr);
  free(expected);

  free_qureg(&qr);  

  return;
}

void test_nshots(void) {
  const size_t NMEASURE = NQUBITS;
  const cstate CR_INIT_VAL = -1;
  size_t nshots;
  cstate * cr, * expected;

  nshots = 0;
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS,
    sm_qrun(zero_init_full_qft, NULL, NQUBITS, NULL, NMEASURE, nshots)
  );

  qubit * qr = NULL;
  alloc_qureg(&qr, NQUBITS);

  nshots = 4;
  cr = (cstate*) malloc(nshots*NMEASURE*sizeof(cstate));
  init_creg(nshots*NMEASURE, CR_INIT_VAL, cr);
  expected = (cstate*) malloc(nshots*NMEASURE*sizeof(cstate));
  init_creg(nshots*NMEASURE, 1, expected);

  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS,
    sm_qrun(zero_init_full_qft, qr, NQUBITS, cr, NMEASURE, nshots)
  );
  TEST_ASSERT_INT8_ARRAY_WITHIN(1, expected, cr, NMEASURE*nshots); 

  free_qureg(&qr);
  free(cr);
  free(expected);

  return;
}

void test_bad_inputs(void) {
  const size_t NMEASURE = NQUBITS;
  const size_t NSHOTS = 2;
  const cstate CR_INIT_VAL = -1;
  cstate * cr, * empty_cr;
  qubit * qr = NULL;

  cr = (cstate*) malloc(NMEASURE*NSHOTS*sizeof(cstate));

  init_creg(NMEASURE*NSHOTS, CR_INIT_VAL, cr);

  // NULL or unregistered quantum kernel
  TEST_ASSERT_EQUAL_INT(CQ_ERROR,
    sm_qrun(NULL, NULL, NQUBITS, NULL, NMEASURE, NSHOTS)
  );
  TEST_ASSERT_EQUAL_INT(CQ_ERROR,
    sm_qrun(unregistered_kernel, NULL, NQUBITS, NULL, NMEASURE, NSHOTS)
  );

  // NULL qureg
  TEST_ASSERT_EQUAL_INT(CQ_ERROR,
    sm_qrun(all_site_hadamard, NULL, NQUBITS, cr, NMEASURE, NSHOTS)
  );
  TEST_ASSERT_EACH_EQUAL_INT8(CR_INIT_VAL, cr, NMEASURE*NSHOTS);
  
  alloc_qureg(&qr, NQUBITS);
  free_qureg(&qr);
  TEST_ASSERT_EQUAL_INT(CQ_ERROR,
    sm_qrun(all_site_hadamard, qr, NQUBITS, cr, NMEASURE, NSHOTS)
  );

  // NULL creg
  alloc_qureg(&qr, NQUBITS);
  TEST_ASSERT_EQUAL_INT(CQ_ERROR,
    sm_qrun(all_site_hadamard, qr, NQUBITS, NULL, NMEASURE, NSHOTS)
  );

  free_qureg(&qr);
  free(cr);

  return;
}