#include <stdio.h>
#include "unity.h"
#include "datatypes.h"
#include "host_ops.h"
#include "test_host_resource_management.h"

qubit * g_qr = NULL;

// run before each test
void setUp(void) {
  return;
}

// run after each test 
void tearDown(void) {
  return;
}

void test_first_alloc_qubit(void) {
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, alloc_qubit(&g_qr));
  TEST_ASSERT_NOT_NULL(g_qr);
  return;
}

void test_first_free_qubit(void) {
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, free_qubit(&g_qr));
  TEST_ASSERT_NULL(g_qr);
  return;
}

void test_first_alloc_qureg(void) {
  const size_t NQ = 1;
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, alloc_qureg(&g_qr, NQ));
  TEST_ASSERT_NOT_NULL(g_qr);
  return;
}

void test_first_free_qureg(void) {
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, free_qureg(&g_qr));
  TEST_ASSERT_NULL(g_qr);
  return;
}

void test_alloc_and_free_qureg(void) {
  qubit * qr = NULL;
  size_t nqubits;
  size_t idx;
  char msg[12];
  
  nqubits = 1;
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, alloc_qubit(&qr));
  TEST_ASSERT_NOT_NULL(qr);
  TEST_ASSERT_EQUAL_size_t(0, qr->offset);
  TEST_ASSERT_EQUAL_size_t(nqubits, qr->N);
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, free_qubit(&qr));
  TEST_ASSERT_NULL(qr);
  
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, alloc_qureg(&qr, nqubits));
  TEST_ASSERT_NOT_NULL(qr);
  TEST_ASSERT_EQUAL_size_t(0, qr[0].offset);
  TEST_ASSERT_EQUAL_size_t(nqubits, qr[0].N);
  TEST_ASSERT_EQUAL(CQ_SUCCESS, free_qureg(&qr));
  TEST_ASSERT_NULL(qr); 
  
  nqubits = 2; 
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, alloc_qureg(&qr, nqubits));
  TEST_ASSERT_NOT_NULL(qr);
  for (idx = 0; idx < nqubits; ++idx) {
    sprintf(msg, "idx = %lu", idx);
    TEST_ASSERT_EQUAL_size_t_MESSAGE(idx, qr[idx].offset, msg);
    TEST_ASSERT_EQUAL_size_t_MESSAGE(nqubits, qr[idx].N, msg);
  }
  TEST_ASSERT_EQUAL(CQ_SUCCESS, free_qureg(&qr));
  TEST_ASSERT_NULL(qr);
 
  nqubits = 3;
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, alloc_qureg(&qr, nqubits));
  TEST_ASSERT_NOT_NULL(qr);  
  for (idx = 0; idx < nqubits; ++idx) {
    sprintf(msg, "idx = %lu", idx);
    TEST_ASSERT_EQUAL_size_t_MESSAGE(idx, qr[idx].offset, msg);
    TEST_ASSERT_EQUAL_size_t_MESSAGE(nqubits, qr[idx].N, msg);
  }
  TEST_ASSERT_EQUAL(CQ_SUCCESS, free_qureg(&qr));
  TEST_ASSERT_NULL(qr);
 
  return;
}

void test_realloc_qureg(void) {
  qubit * qr = NULL;
  const size_t NQ = 3;

  alloc_qureg(&qr, NQ);
  // Note, this leaks! Not the Qureg, but the qubit register
  TEST_ASSERT_EQUAL_INT(CQ_WARNING, alloc_qureg(&qr, NQ));
  free_qureg(&qr);

  return;
}

void test_double_free_qureg(void) {
  qubit * qr = NULL;
  const size_t NQ = 3;

  alloc_qubit(&qr);
  free_qubit(&qr);
  TEST_ASSERT_EQUAL_INT(CQ_WARNING, free_qubit(&qr));

  alloc_qureg(&qr, NQ);
  free_qureg(&qr);
  TEST_ASSERT_EQUAL_INT(CQ_WARNING, free_qureg(&qr));

  return;
}
