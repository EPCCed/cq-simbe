#include "unity.h"
#include "datatypes.h"
#include "utils.h"
#include "test_host_utils.h"

void setUp(void) {
  return;
}

void tearDown(void) {
  return;
}

void test_init_creg(void) {
  cstate init_val = -1;
  cstate len_one;
  cstate len_two[2];
  cstate len_ten[10];

  init_creg(1, init_val, &len_one);
  TEST_ASSERT_EQUAL_INT16(init_val, len_one);
  init_creg(2, init_val, len_two);
  TEST_ASSERT_EACH_EQUAL_INT16(init_val, len_two, 2);
  init_creg(2, init_val, len_ten);
  TEST_ASSERT_EACH_EQUAL_INT16(init_val, len_ten, 2);
  init_creg(10, init_val, len_ten);
  TEST_ASSERT_EACH_EQUAL_INT16(init_val, len_ten, 10);

  init_val = 10;
  init_creg(10, init_val, len_ten);
  TEST_ASSERT_EACH_EQUAL_INT16(init_val, len_ten, 10);

  return;
}