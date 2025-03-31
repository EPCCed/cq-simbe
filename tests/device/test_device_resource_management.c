#include <stdbool.h>
#include "unity.h"
#include "datatypes.h"
#include "src/device/resources.h"
#include "test_device_resource_management.h"

void setUp(void) {
  return;
}

void tearDown(void) {
  return;
}

void test_init_qregistry(void) {
  char msg[12];

  init_qregistry();
  // There should be 0 registers initially
  TEST_ASSERT_EQUAL_size_t(0, qregistry.num_registers);
  // Every slot should be shown as available
  for (size_t i = 0; i < __DEVICE_MAX_NUM_QUREGS__; ++i) {
    sprintf(msg, "idx = %lu", i);
    TEST_ASSERT_TRUE_MESSAGE(qregistry.available[i], msg);
  }
  return;
}

void test_next_available_slot(void) {

  return;
}

void test_device_alloc_and_dealloc_qureg(void) {
  device_alloc_params q1 = {
    .NQUBITS = 1,
    .qregistry_idx = 0,
    .status = CQ_ERROR
  };

  device_alloc_params q2 = {
    .NQUBITS = 2,
    .qregistry_idx = 0,
    .status = CQ_ERROR
  };

  device_alloc_params q3 = {
    .NQUBITS = 3,
    .qregistry_idx = 0,
    .status = CQ_ERROR
  };

  // Just check we're doing this in the right order, and 
  // qregistry is freshly initialised
  TEST_ASSERT_EQUAL_size_t(0, qregistry.num_registers);

  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, device_alloc_qureg(&q1));
  TEST_ASSERT_EQUAL_size_t(1, qregistry.num_registers);
  // If the allocation didn't work, this should segfault!
  TEST_ASSERT_EQUAL(q1.NQUBITS, qregistry.registers[0].numQubits);

  return;
}

void test_clear_qregistry(void) {
  
  return;
}
