#include "unity.h"
#include "env.h"

// runs before each test
void setUp(void) {
  return;
}

// runs after each test
void tearDown(void) {
  cq_finalise(0);
  return;
}

void test_initialisation_return_zero(void) {
  TEST_ASSERT_EQUAL_INT(0, cq_init(0));
}