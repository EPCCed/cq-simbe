#include "unity.h"
#include "datatypes.h"
#include "tests/test_qkerns.h"
#include "kernel_utils.h"
#include "test_kernel_utils.h"

void setUp(void) {
  return;
}

void tearDown(void) {
  return;
}

void test_register_qkern(void) {
  TEST_ASSERT_EQUAL_size_t(0, qk_reg.next_available_slot);
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, register_qkern(zero_init_full_qft));
  TEST_ASSERT_EQUAL_PTR(zero_init_full_qft, qk_reg.qkernels[0].fn);
  TEST_ASSERT_EQUAL_size_t(1, qk_reg.next_available_slot);
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, register_qkern(plus_init_full_qft));
  TEST_ASSERT_EQUAL_PTR(plus_init_full_qft, qk_reg.qkernels[1].fn);
  TEST_ASSERT_EQUAL_size_t(2, qk_reg.next_available_slot);
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, register_qkern(all_site_hadamard));
  TEST_ASSERT_EQUAL_PTR(all_site_hadamard, qk_reg.qkernels[2].fn);
  TEST_ASSERT_EQUAL_size_t(3, qk_reg.next_available_slot);
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, register_qkern(only_measure_first_site));
  TEST_ASSERT_EQUAL_PTR(only_measure_first_site, qk_reg.qkernels[3].fn);
  TEST_ASSERT_EQUAL_size_t(4, qk_reg.next_available_slot);
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, register_qkern(no_measure_qkern));
  TEST_ASSERT_EQUAL_PTR(no_measure_qkern, qk_reg.qkernels[4].fn);
  TEST_ASSERT_EQUAL_size_t(5, qk_reg.next_available_slot);

  // This should not break anything, but definitely isn't what you intended
  TEST_ASSERT_EQUAL_INT(CQ_WARNING, register_qkern(zero_init_full_qft));
  TEST_ASSERT_NULL(qk_reg.qkernels[5].fn);
  TEST_ASSERT_EQUAL_size_t(5, qk_reg.next_available_slot);

  // These should break things
  TEST_ASSERT_EQUAL_INT(CQ_ERROR, register_qkern(unregistered_kernel));
  TEST_ASSERT_EQUAL_size_t(5, qk_reg.next_available_slot);

  qk_reg.next_available_slot = __CQ_MAX_NUM_QKERN__;
  TEST_ASSERT_EQUAL_INT(CQ_ERROR, register_qkern(zero_init_full_qft));
  TEST_ASSERT_EQUAL_size_t(__CQ_MAX_NUM_QKERN__, qk_reg.next_available_slot);
  // reset our vandalism of the qkern registry as there's no function to clear
  // it currently
  qk_reg.next_available_slot = 5;
  TEST_ASSERT_EQUAL_INT(5, qk_reg.next_available_slot);

  TEST_ASSERT_EQUAL_INT(CQ_ERROR, register_qkern(NULL));

  TEST_ASSERT_EQUAL_INT(CQ_ERROR, register_qkern(overly_long_qkern_name));

  return;
}

void test_find_qkern_pointer(void) {
  char msg[128 + __CQ_MAX_QKERN_NAME_LENGTH__];
  qkern qk = NULL;

  for (size_t i = 0; i < qk_reg.next_available_slot; ++i) {
    sprintf(msg, "i = %zu, fname = %s", i, qk_reg.qkernels[i].fname);
    TEST_ASSERT_EQUAL_INT_MESSAGE(CQ_SUCCESS, find_qkern_pointer(qk_reg.qkernels[i].fname, &qk), msg);
    TEST_ASSERT_EQUAL_PTR_MESSAGE(qk_reg.qkernels[i].fn, qk, msg);
    qk = NULL;
  }

  TEST_ASSERT_EQUAL_INT(CQ_ERROR, find_qkern_pointer("unregistered_kernel", &qk));
  TEST_ASSERT_NULL(qk);

  return;
}

void test_find_qkern_name(void) {
  char msg[64];
  char * fname = NULL;

  for (size_t i = 0; i < qk_reg.next_available_slot; ++i) {
    sprintf(msg, "i = %zu, fn = %p", i, (void*) qk_reg.qkernels[i].fn);
    TEST_ASSERT_EQUAL_INT_MESSAGE(CQ_SUCCESS, find_qkern_name(qk_reg.qkernels[i].fn, &fname), msg);
    TEST_ASSERT_EQUAL_STRING_MESSAGE(qk_reg.qkernels[i].fname, fname, msg);
    fname = NULL;
  }

  TEST_ASSERT_EQUAL_INT(CQ_ERROR, find_qkern_name(unregistered_kernel, &fname));
  TEST_ASSERT_NULL(fname);

  return;
}

void test_init_and_finalise_exec_handle(void) {
  const size_t NQUBITS = 8;
  const size_t NSHOTS = 100;
  const size_t NMEASURE = 4;
  cq_exec eh;

  init_exec_handle(NQUBITS, NSHOTS, NMEASURE, &eh);
  TEST_ASSERT(eh.exec_init);
  TEST_ASSERT_FALSE(eh.complete);
  TEST_ASSERT_EQUAL_INT(CQ_ERROR, eh.status);
  TEST_ASSERT_EQUAL_size_t(NQUBITS, eh.nqubits);
  TEST_ASSERT_EQUAL_size_t(0, eh.completed_shots);
  TEST_ASSERT_EQUAL_size_t(NSHOTS, eh.expected_shots);
  TEST_ASSERT_EQUAL_size_t(NMEASURE, eh.nmeasure);
  TEST_ASSERT_NULL(eh.fname);
  TEST_ASSERT_NULL(eh.qreg);
  TEST_ASSERT_NULL(eh.creg);
  TEST_ASSERT_NULL(eh.params);

  finalise_exec_handle(&eh);
  TEST_ASSERT_FALSE(eh.exec_init);
  TEST_ASSERT_FALSE(eh.complete);
  TEST_ASSERT_EQUAL_INT(CQ_ERROR, eh.status);
  TEST_ASSERT_EQUAL_size_t(0, eh.completed_shots);
  TEST_ASSERT_EQUAL_size_t(NSHOTS, eh.expected_shots);
  
  return;
}