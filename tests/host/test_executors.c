#include <stdlib.h>
#include "unity.h"
#include "host_ops.h"
#include "utils.h"
#include "tests/test_qkerns.h"
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
  cq_exec eh;

  cstate * cr;
  cr = (cstate*) malloc(NMEASURE*NSHOTS*sizeof(cstate));

  cstate * expected;
  expected = (cstate*) malloc(NMEASURE*NSHOTS*sizeof(cstate));
  init_creg(NMEASURE*NSHOTS, 1, expected);

  qubit * qr = NULL;
  alloc_qureg(&qr, NQUBITS);

  init_creg(NMEASURE, CR_INIT_VAL, cr);
  TEST_ASSERT_EACH_EQUAL_INT16(CR_INIT_VAL, cr, NMEASURE);
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS,
    s_qrun(zero_init_full_qft, qr, NQUBITS, cr, NMEASURE)
  );
  TEST_ASSERT_INT16_ARRAY_WITHIN(1, expected, cr, NMEASURE);

  init_creg(NMEASURE, CR_INIT_VAL, cr);
  TEST_ASSERT_EACH_EQUAL_INT16(CR_INIT_VAL, cr, NMEASURE);
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS,
    a_qrun(zero_init_full_qft, qr, NQUBITS, cr, NMEASURE, &eh)
  );
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, wait_qrun(&eh));
  TEST_ASSERT_FALSE(eh.exec_init);
  TEST_ASSERT(eh.complete);
  TEST_ASSERT_EQUAL_size_t(1, eh.completed_shots);
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, eh.status);
  TEST_ASSERT_INT16_ARRAY_WITHIN(1, expected, cr, NMEASURE);

  init_creg(NMEASURE*NSHOTS, CR_INIT_VAL, cr);
  TEST_ASSERT_EACH_EQUAL_INT16(CR_INIT_VAL, cr, NMEASURE*NSHOTS);
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, 
    sm_qrun(zero_init_full_qft, qr, NQUBITS, cr, NMEASURE, NSHOTS)
  );
  // tests that all elements with cr are in the range 0-2 i.e. not -1
  TEST_ASSERT_INT16_ARRAY_WITHIN(1, expected, cr, NMEASURE*NSHOTS);

  init_creg(NMEASURE*NSHOTS, CR_INIT_VAL, cr);
  TEST_ASSERT_EACH_EQUAL_INT16(CR_INIT_VAL, cr, NMEASURE*NSHOTS);
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, 
    am_qrun(zero_init_full_qft, qr, NQUBITS, cr, NMEASURE, NSHOTS, &eh)
  );
  TEST_ASSERT(eh.exec_init);
  TEST_ASSERT_EQUAL_INT(CQ_ERROR, eh.status);
  TEST_ASSERT_EQUAL_size_t(NSHOTS, eh.expected_shots);
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, sync_qrun(&eh));
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, wait_qrun(&eh));
  TEST_ASSERT_FALSE(eh.exec_init);
  TEST_ASSERT(eh.complete);
  TEST_ASSERT_EQUAL_size_t(NSHOTS, eh.completed_shots);
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, eh.status);
  TEST_ASSERT_INT16_ARRAY_WITHIN(1, expected, cr, NMEASURE*NSHOTS);

  init_creg(NMEASURE, CR_INIT_VAL, cr);
  TEST_ASSERT_EACH_EQUAL_INT16(CR_INIT_VAL, cr, NMEASURE);
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS,
    s_qrun(plus_init_full_qft, qr, NQUBITS, cr, NMEASURE)
  );
  TEST_ASSERT_INT16_ARRAY_WITHIN(1, expected, cr, NMEASURE);

  init_creg(NMEASURE, CR_INIT_VAL, cr);
  TEST_ASSERT_EACH_EQUAL_INT16(CR_INIT_VAL, cr, NMEASURE);
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS,
    a_qrun(plus_init_full_qft, qr, NQUBITS, cr, NMEASURE, &eh)
  );
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, wait_qrun(&eh));
  TEST_ASSERT_FALSE(eh.exec_init);
  TEST_ASSERT(eh.complete);
  TEST_ASSERT_EQUAL_size_t(1, eh.completed_shots);
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, eh.status);
  TEST_ASSERT_INT16_ARRAY_WITHIN(1, expected, cr, NMEASURE);

  init_creg(NMEASURE*NSHOTS, CR_INIT_VAL, cr);
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS,
    sm_qrun(plus_init_full_qft, qr, NQUBITS, cr, NMEASURE, NSHOTS)
  );
  TEST_ASSERT_INT16_ARRAY_WITHIN(1, expected, cr, NMEASURE*NSHOTS);

  init_creg(NMEASURE*NSHOTS, CR_INIT_VAL, cr);
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS,
    am_qrun(plus_init_full_qft, qr, NQUBITS, cr, NMEASURE, NSHOTS, 
      &eh)
  );
  TEST_ASSERT(eh.exec_init);
  TEST_ASSERT_EQUAL_INT(CQ_ERROR, eh.status);
  TEST_ASSERT_EQUAL_size_t(NSHOTS, eh.expected_shots);
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, sync_qrun(&eh));
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, wait_qrun(&eh));
  TEST_ASSERT_FALSE(eh.exec_init);
  TEST_ASSERT(eh.complete);
  TEST_ASSERT_EQUAL_size_t(NSHOTS, eh.completed_shots);
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, eh.status);
  TEST_ASSERT_INT16_ARRAY_WITHIN(1, expected, cr, NMEASURE*NSHOTS);

  init_creg(NMEASURE, CR_INIT_VAL, cr);
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS,
    s_qrun(all_site_hadamard, qr, NQUBITS, cr, NMEASURE)
  );
  TEST_ASSERT_INT16_ARRAY_WITHIN(1, expected, cr, NMEASURE);

  init_creg(NMEASURE, CR_INIT_VAL, cr);
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS,
    a_qrun(all_site_hadamard, qr, NQUBITS, cr, NMEASURE, &eh)
  );
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, wait_qrun(&eh));
  TEST_ASSERT_FALSE(eh.exec_init);
  TEST_ASSERT(eh.complete);
  TEST_ASSERT_EQUAL_size_t(1, eh.completed_shots);
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, eh.status);
  TEST_ASSERT_INT16_ARRAY_WITHIN(1, expected, cr, NMEASURE);

  init_creg(NMEASURE*NSHOTS, CR_INIT_VAL, cr);
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS,
    sm_qrun(all_site_hadamard, qr, NQUBITS, cr, NMEASURE, NSHOTS)
  );
  TEST_ASSERT_INT16_ARRAY_WITHIN(1, expected, cr, NMEASURE*NSHOTS);

  init_creg(NMEASURE*NSHOTS, CR_INIT_VAL, cr);
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS,
    am_qrun(all_site_hadamard, qr, NQUBITS, cr, NMEASURE, NSHOTS, &eh)
  );
  TEST_ASSERT(eh.exec_init);
  TEST_ASSERT_EQUAL_INT(CQ_ERROR, eh.status);
  TEST_ASSERT_EQUAL_size_t(NSHOTS, eh.expected_shots);
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, sync_qrun(&eh));
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, wait_qrun(&eh));
  TEST_ASSERT_FALSE(eh.exec_init);
  TEST_ASSERT(eh.complete);
  TEST_ASSERT_EQUAL_size_t(NSHOTS, eh.completed_shots);
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, eh.status);
  TEST_ASSERT_INT16_ARRAY_WITHIN(1, expected, cr, NMEASURE*NSHOTS);

  init_creg(NMEASURE, CR_INIT_VAL, cr);
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS,
    s_qrun(only_measure_first_site, qr, NQUBITS, cr, 1)
  );
  TEST_ASSERT_INT16_WITHIN(1, 1, cr[0]);
  TEST_ASSERT_EACH_EQUAL_INT16(CR_INIT_VAL, cr+1, NMEASURE-1);

  init_creg(NMEASURE, CR_INIT_VAL, cr);
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS,
    a_qrun(only_measure_first_site, qr, NQUBITS, cr, 1, &eh)
  );
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, wait_qrun(&eh));
  TEST_ASSERT_FALSE(eh.exec_init);
  TEST_ASSERT(eh.complete);
  TEST_ASSERT_EQUAL_size_t(1, eh.completed_shots);
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, eh.status);
  TEST_ASSERT_INT16_WITHIN(1, 1, cr[0]);
  TEST_ASSERT_EACH_EQUAL_INT16(CR_INIT_VAL, cr+1, NMEASURE-1);

  init_creg(NMEASURE*NSHOTS, CR_INIT_VAL, cr);
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS,
    // we use NMEASURE=1 here
    sm_qrun(only_measure_first_site, qr, NQUBITS, cr, 1, NSHOTS)
  );
  TEST_ASSERT_INT16_ARRAY_WITHIN(1, expected, cr, NSHOTS);
  TEST_ASSERT_EACH_EQUAL_INT16(CR_INIT_VAL, cr+NSHOTS, (NMEASURE-1)*NSHOTS);

  init_creg(NMEASURE*NSHOTS, CR_INIT_VAL, cr);
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS,
    // we use NMEASURE=1 here
    am_qrun(only_measure_first_site, qr, NQUBITS, cr, 1, NSHOTS, &eh)
  );
  TEST_ASSERT(eh.exec_init);
  TEST_ASSERT_EQUAL_INT(CQ_ERROR, eh.status);
  TEST_ASSERT_EQUAL_size_t(NSHOTS, eh.expected_shots);
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, sync_qrun(&eh));
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, wait_qrun(&eh));
  TEST_ASSERT_FALSE(eh.exec_init);
  TEST_ASSERT(eh.complete);
  TEST_ASSERT_EQUAL_size_t(NSHOTS, eh.completed_shots);
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, eh.status);
  TEST_ASSERT_INT16_ARRAY_WITHIN(1, expected, cr, NSHOTS);
  TEST_ASSERT_EACH_EQUAL_INT16(CR_INIT_VAL, cr+NSHOTS, (NMEASURE-1)*NSHOTS);

  // no measurements here!
  init_creg(NMEASURE, CR_INIT_VAL, cr);
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS,
    s_qrun(no_measure_qkern, qr, NQUBITS, cr, NMEASURE)
  );
  TEST_ASSERT_EACH_EQUAL_INT16(CR_INIT_VAL, cr, NSHOTS*NMEASURE);

  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, 
    a_qrun(no_measure_qkern, qr, NQUBITS, cr, NMEASURE, &eh)
  );
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, wait_qrun(&eh));
  TEST_ASSERT_FALSE(eh.exec_init);
  TEST_ASSERT(eh.complete);
  TEST_ASSERT_EQUAL_size_t(NSHOTS, eh.completed_shots);
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, eh.status);
  TEST_ASSERT_EACH_EQUAL_INT16(CR_INIT_VAL, cr, NMEASURE);

  init_creg(NSHOTS*NMEASURE, CR_INIT_VAL, cr);
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS,
    sm_qrun(no_measure_qkern, qr, NQUBITS, cr, NMEASURE, NSHOTS)
  );
  TEST_ASSERT_EACH_EQUAL_INT16(CR_INIT_VAL, cr, NSHOTS*NMEASURE);

  init_creg(NSHOTS*NMEASURE, CR_INIT_VAL, cr);
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS,
    am_qrun(no_measure_qkern, qr, NQUBITS, cr, NMEASURE, NSHOTS, &eh)
  );
  TEST_ASSERT(eh.exec_init);
  TEST_ASSERT_EQUAL_INT(CQ_ERROR, eh.status);
  TEST_ASSERT_EQUAL_size_t(NSHOTS, eh.expected_shots);
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, sync_qrun(&eh));
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, wait_qrun(&eh));
  TEST_ASSERT_FALSE(eh.exec_init);
  TEST_ASSERT(eh.complete);
  TEST_ASSERT_EQUAL_size_t(NSHOTS, eh.completed_shots);
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, eh.status);
  TEST_ASSERT_EACH_EQUAL_INT16(CR_INIT_VAL, cr, NSHOTS*NMEASURE);

  free_qureg(&qr);
  free(cr);

  return;
}

void test_nmeasure(void) {
  const size_t NSHOTS = 2;
  const cstate CR_INIT_VAL = -1;
  size_t nmeasure;
  cstate * cr, * expected;
  cq_exec eh;
  
  qubit * qr = NULL;
  alloc_qureg(&qr, NQUBITS);

  nmeasure = 0;
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS,
    s_qrun(no_measure_qkern, qr, NQUBITS, NULL, nmeasure)
  );
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS,
    a_qrun(no_measure_qkern, qr, NQUBITS, cr, nmeasure, &eh)
  );
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS,
    wait_qrun(&eh)
  );
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS,
    sm_qrun(no_measure_qkern, qr, NQUBITS, NULL, nmeasure, NSHOTS)
  );
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS,
    am_qrun(no_measure_qkern, qr, NQUBITS, cr, nmeasure, NSHOTS, &eh)
  );
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS,
    wait_qrun(&eh)
  );

  nmeasure = 1;
  cr = (cstate*) malloc(nmeasure*NSHOTS*sizeof(cstate)); 
  expected = (cstate*) malloc(nmeasure*NSHOTS*sizeof(cstate));
  
  init_creg(nmeasure*NSHOTS, 1, expected);
  
  init_creg(nmeasure, CR_INIT_VAL, cr);
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS,
    s_qrun(only_measure_first_site, qr, NQUBITS, cr, nmeasure)
  );
  TEST_ASSERT_INT16_WITHIN(1, 1, cr[0]);

  init_creg(nmeasure, CR_INIT_VAL, cr);
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS,
    a_qrun(only_measure_first_site, qr, NQUBITS, cr, nmeasure, &eh)
  );
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, wait_qrun(&eh));
  TEST_ASSERT_INT16_WITHIN(1, 1, cr[0]);

  init_creg(nmeasure*NSHOTS, CR_INIT_VAL, cr);
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS,
    sm_qrun(only_measure_first_site, qr, NQUBITS, cr, nmeasure, NSHOTS)
  );
  TEST_ASSERT_INT16_ARRAY_WITHIN(1, expected, cr, nmeasure*NSHOTS);
 
  init_creg(nmeasure*NSHOTS, CR_INIT_VAL, cr);
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS,
    am_qrun(only_measure_first_site, qr, NQUBITS, cr, nmeasure, NSHOTS, &eh)
  );
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, wait_qrun(&eh));
  TEST_ASSERT_INT16_ARRAY_WITHIN(1, expected, cr, nmeasure*NSHOTS);
  
  free(cr);
  free(expected);

  nmeasure = NQUBITS;
  cr = (cstate*) malloc(nmeasure*NSHOTS*sizeof(cstate)); 
  expected = (cstate*) malloc(nmeasure*NSHOTS*sizeof(cstate));
  init_creg(nmeasure*NSHOTS, 1, expected);
  
  init_creg(nmeasure, CR_INIT_VAL, cr);
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS,
    s_qrun(all_site_hadamard, qr, NQUBITS, cr, nmeasure)
  );
  TEST_ASSERT_INT16_ARRAY_WITHIN(1, expected, cr, nmeasure);

  init_creg(nmeasure, CR_INIT_VAL, cr);
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS,
    a_qrun(all_site_hadamard, qr, NQUBITS, cr, nmeasure, &eh)
  );
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, wait_qrun(&eh));
  TEST_ASSERT_INT16_ARRAY_WITHIN(1, expected, cr, nmeasure); 

  init_creg(nmeasure*NSHOTS, CR_INIT_VAL, cr);
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS,
    sm_qrun(all_site_hadamard, qr, NQUBITS, cr, nmeasure, NSHOTS)
  );
  TEST_ASSERT_INT16_ARRAY_WITHIN(1, expected, cr, nmeasure*NSHOTS);
  
  init_creg(nmeasure*NSHOTS, CR_INIT_VAL, cr);
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS,
    am_qrun(all_site_hadamard, qr, NQUBITS, cr, nmeasure, NSHOTS, &eh)
  );
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, wait_qrun(&eh));
  TEST_ASSERT_INT16_ARRAY_WITHIN(1, expected, cr, nmeasure*NSHOTS);
   
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
  cq_exec eh;

  nshots = 0;
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS,
    sm_qrun(zero_init_full_qft, NULL, NQUBITS, NULL, NMEASURE, nshots)
  );

  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS,
    am_qrun(zero_init_full_qft, NULL, NQUBITS, NULL, NMEASURE, nshots, &eh)
  );
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, wait_qrun(&eh));

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
  TEST_ASSERT_INT16_ARRAY_WITHIN(1, expected, cr, NMEASURE*nshots); 

  init_creg(nshots*NMEASURE, CR_INIT_VAL, cr);

  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS,
    am_qrun(zero_init_full_qft, qr, NQUBITS, cr, NMEASURE, nshots, &eh)
  );
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, wait_qrun(&eh));
  TEST_ASSERT_INT16_ARRAY_WITHIN(1, expected, cr, NMEASURE*nshots); 

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
  cq_exec eh;

  cr = (cstate*) malloc(NMEASURE*NSHOTS*sizeof(cstate));

  init_creg(NMEASURE*NSHOTS, CR_INIT_VAL, cr);

  // NULL or unregistered quantum kernel
  TEST_ASSERT_EQUAL_INT(CQ_ERROR,
    s_qrun(NULL, NULL, NQUBITS, NULL, NMEASURE)
  );
  TEST_ASSERT_EQUAL_INT(CQ_ERROR,
    s_qrun(unregistered_kernel, NULL, NQUBITS, NULL, NMEASURE)
  );
  TEST_ASSERT_EQUAL_INT(CQ_ERROR,
    sm_qrun(NULL, NULL, NQUBITS, NULL, NMEASURE, NSHOTS)
  );
  TEST_ASSERT_EQUAL_INT(CQ_ERROR,
    sm_qrun(unregistered_kernel, NULL, NQUBITS, NULL, NMEASURE, NSHOTS)
  );
  TEST_ASSERT_EQUAL_INT(CQ_ERROR,
    a_qrun(NULL, NULL, NQUBITS, NULL, NMEASURE, &eh)
  );
  TEST_ASSERT_EQUAL_INT(CQ_ERROR,
    a_qrun(unregistered_kernel, NULL, NQUBITS, NULL, NMEASURE, &eh)
  );
  TEST_ASSERT_EQUAL_INT(CQ_ERROR,
    am_qrun(NULL, NULL, NQUBITS, NULL, NMEASURE, NSHOTS, &eh)
  );
  TEST_ASSERT_EQUAL_INT(CQ_ERROR,
    am_qrun(unregistered_kernel, NULL, NQUBITS, NULL, NMEASURE, NSHOTS, &eh)
  );

  // NULL qureg
  TEST_ASSERT_EQUAL_INT(CQ_ERROR,
    s_qrun(all_site_hadamard, NULL, NQUBITS, cr, NMEASURE)
  );
  TEST_ASSERT_EACH_EQUAL_INT16(CR_INIT_VAL, cr, NMEASURE*NSHOTS);

  TEST_ASSERT_EQUAL_INT(CQ_ERROR,
    sm_qrun(all_site_hadamard, NULL, NQUBITS, cr, NMEASURE, NSHOTS)
  );
  TEST_ASSERT_EACH_EQUAL_INT16(CR_INIT_VAL, cr, NMEASURE*NSHOTS);

  TEST_ASSERT_EQUAL_INT(CQ_ERROR,
    a_qrun(all_site_hadamard, NULL, NQUBITS, cr, NMEASURE, &eh)
  );
  TEST_ASSERT_EQUAL_INT(CQ_ERROR, wait_qrun(&eh));
  TEST_ASSERT_EACH_EQUAL_INT16(CR_INIT_VAL, cr, NMEASURE*NSHOTS);

  TEST_ASSERT_EQUAL_INT(CQ_ERROR, 
    am_qrun(all_site_hadamard, NULL, NQUBITS, cr, NMEASURE, NSHOTS, &eh)
  );
  TEST_ASSERT_EQUAL_INT(CQ_ERROR, wait_qrun(&eh));
  TEST_ASSERT_EACH_EQUAL_INT16(CR_INIT_VAL, cr, NMEASURE*NSHOTS);

  alloc_qureg(&qr, NQUBITS);
  free_qureg(&qr);
  TEST_ASSERT_EQUAL_INT(CQ_ERROR,
    sm_qrun(all_site_hadamard, qr, NQUBITS, cr, NMEASURE, NSHOTS)
  );

  // NULL creg
  alloc_qureg(&qr, NQUBITS);
  TEST_ASSERT_EQUAL_INT(CQ_ERROR,
    s_qrun(all_site_hadamard, qr, NQUBITS, NULL, NMEASURE)
  );
  TEST_ASSERT_EQUAL_INT(CQ_ERROR,
    a_qrun(all_site_hadamard, qr, NQUBITS, NULL, NMEASURE, &eh)
  );
  TEST_ASSERT_EQUAL_INT(CQ_ERROR, wait_qrun(&eh));
  TEST_ASSERT_EQUAL_INT(CQ_ERROR,
    sm_qrun(all_site_hadamard, qr, NQUBITS, NULL, NMEASURE, NSHOTS)
  );
  TEST_ASSERT_EQUAL_INT(CQ_ERROR,
    am_qrun(all_site_hadamard, qr, NQUBITS, NULL, NMEASURE, NSHOTS, &eh)
  );
  TEST_ASSERT_EQUAL_INT(CQ_ERROR, wait_qrun(&eh));

  free_qureg(&qr);
  free(cr);

  return;
}

void test_kernel_abort(void) {
  const size_t NMEASURE = NQUBITS;
  const size_t NSHOTS = 10;
  const size_t EXP_SHOTS = 4;
  const cstate CR_INIT_VAL = -1;
  const int CUSTOM_STATUS = 666;
  cstate * cr;
  qubit * qr;
  cq_exec eh;

  cr = (cstate *) malloc(NMEASURE * NSHOTS * sizeof(cstate));
  alloc_qureg(&qr, NQUBITS);

  init_creg(NMEASURE*NSHOTS, CR_INIT_VAL, cr);
  TEST_ASSERT_EQUAL_INT(CQ_ERROR,
    s_qrun(immediate_qabort, qr, NQUBITS, cr, NMEASURE)
  );
  TEST_ASSERT_EACH_EQUAL_INT16(CR_INIT_VAL, cr, NMEASURE * NSHOTS);

  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS,
    a_qrun(immediate_qabort, qr, NQUBITS, cr, NMEASURE, &eh)
  );
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, wait_qrun(&eh));
  TEST_ASSERT_EQUAL_INT(CQ_ERROR, eh.status);
  TEST_ASSERT_EQUAL_size_t(1, eh.completed_shots);
  TEST_ASSERT_EACH_EQUAL_INT16(CR_INIT_VAL, cr, NMEASURE * NSHOTS);

  TEST_ASSERT_EQUAL_INT(CQ_ERROR,
    sm_qrun(immediate_qabort, qr, NQUBITS, cr, NMEASURE, NSHOTS)
  );
  TEST_ASSERT_EACH_EQUAL_INT16(CR_INIT_VAL, cr, NSHOTS * NMEASURE);

  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS,
    am_qrun(successful_qabort, qr, NQUBITS, cr, NMEASURE, NSHOTS, &eh)
  );
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, wait_qrun(&eh));
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, eh.status);
  TEST_ASSERT_EQUAL_size_t(EXP_SHOTS, eh.completed_shots);
  TEST_ASSERT_EACH_EQUAL_INT16(1, cr, EXP_SHOTS * NMEASURE);
  TEST_ASSERT_EACH_EQUAL_INT16(CR_INIT_VAL, cr + EXP_SHOTS*NMEASURE, (NSHOTS-EXP_SHOTS)*NMEASURE);

  init_creg(NMEASURE * NSHOTS, CR_INIT_VAL, cr);
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS,
    am_qrun(cq_error_qabort, qr, NQUBITS, cr, NMEASURE, NSHOTS, &eh)
  );
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, wait_qrun(&eh));
  TEST_ASSERT_EQUAL_INT(CQ_ERROR, eh.status);
  TEST_ASSERT_EQUAL_size_t(EXP_SHOTS, eh.completed_shots);
  TEST_ASSERT_EACH_EQUAL_INT16(1, cr, EXP_SHOTS * NMEASURE);
  TEST_ASSERT_EACH_EQUAL_INT16(CR_INIT_VAL, cr + EXP_SHOTS*NMEASURE, (NSHOTS-EXP_SHOTS)*NMEASURE);

  init_creg(NMEASURE * NSHOTS, CR_INIT_VAL, cr);
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS,
    am_qrun(custom_error_qabort, qr, NQUBITS, cr, NMEASURE, NSHOTS, &eh)
  );
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, wait_qrun(&eh));
  TEST_ASSERT_EQUAL_INT(CUSTOM_STATUS, eh.status);
  TEST_ASSERT_EQUAL_size_t(EXP_SHOTS, eh.completed_shots);
  TEST_ASSERT_EACH_EQUAL_INT16(1, cr, EXP_SHOTS * NMEASURE);
  TEST_ASSERT_EACH_EQUAL_INT16(CR_INIT_VAL, cr + EXP_SHOTS*NMEASURE, (NSHOTS-EXP_SHOTS)*NMEASURE);

  free(cr);
  free_qureg(&qr);

  return;
}