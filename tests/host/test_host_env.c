#include "unity.h"
#include "env.h"
#include "test_host_env.h"

// runs before each test
void setUp(void) {
  return;
}

// runs after each test
void tearDown(void) {
  return;
}

void test_initial_environment_flags(void) {
  TEST_ASSERT_FALSE(cq_env.initialised);
  TEST_ASSERT_FALSE(cq_env.finalised);
  return;
}

void test_init(void) {
  TEST_ASSERT_EQUAL_INT(0, cq_init(1));
  TEST_ASSERT_TRUE(cq_env.initialised);
  TEST_ASSERT_FALSE(cq_env.finalised);
  return;
}

void test_double_init(void) {
  TEST_ASSERT_EQUAL_INT(1, cq_init(1));
  TEST_ASSERT_TRUE(cq_env.initialised);
  TEST_ASSERT_FALSE(cq_env.finalised);
  return;
}

void test_finalise(void) {
  TEST_ASSERT_EQUAL_INT(0, cq_finalise(0));
  TEST_ASSERT_TRUE(cq_env.initialised);
  TEST_ASSERT_TRUE(cq_env.finalised);
}

void test_double_finalise(void) {
  TEST_ASSERT_EQUAL_INT(1, cq_finalise(1));
  TEST_ASSERT_TRUE(cq_env.initialised);
  TEST_ASSERT_TRUE(cq_env.finalised);
  return;
}

void test_reinitialise(void) {
  TEST_ASSERT_EQUAL_INT(-1, cq_init(1));
  TEST_ASSERT_TRUE(cq_env.initialised);
  TEST_ASSERT_TRUE(cq_env.finalised);
  return;
}