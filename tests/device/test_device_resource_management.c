#include <stdbool.h>
#include "unity.h"
#include "datatypes.h"
#include "src/host-device/comms.h"
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
    TEST_ASSERT_NULL_MESSAGE(qregistry.registers[i].cpuAmps, msg);
  }
  return;
}

void test_next_available_slot(void) {
  TEST_ASSERT_EQUAL_INT(0, get_next_available_qregistry_slot());

  qregistry.available[0] = false;
  TEST_ASSERT_EQUAL_INT(1, get_next_available_qregistry_slot());

  qregistry.available[1] = false;
  TEST_ASSERT_EQUAL_INT(2, get_next_available_qregistry_slot());

  qregistry.available[0] = true;
  TEST_ASSERT_EQUAL_INT(0, get_next_available_qregistry_slot());

  // undo previous messing around
  qregistry.available[1] = true;
  qregistry.available[2] = true;

  qregistry.num_registers = __DEVICE_MAX_NUM_QUREGS__;
  TEST_ASSERT_EQUAL_INT(CQ_ERROR, get_next_available_qregistry_slot());
  qregistry.num_registers = 0;

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
  TEST_ASSERT_EQUAL(q1.NQUBITS, qregistry.registers[0].numQubits);
  TEST_ASSERT_NOT_NULL(qregistry.registers[0].cpuAmps);

  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, device_dealloc_qureg(&q1));
  TEST_ASSERT_EQUAL_size_t(0, qregistry.num_registers);

  TEST_ASSERT_EQUAL_INT(CQ_ERROR, device_dealloc_qureg(&q1));
  TEST_ASSERT_EQUAL_size_t(0, qregistry.num_registers);

  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, device_alloc_qureg(&q1));
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, device_alloc_qureg(&q2));
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, device_alloc_qureg(&q3));
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, device_alloc_qureg(&q1));

  TEST_ASSERT_EQUAL_size_t(4, qregistry.num_registers);
  TEST_ASSERT_EQUAL(q1.NQUBITS, qregistry.registers[0].numQubits);
  TEST_ASSERT_NOT_NULL(qregistry.registers[0].cpuAmps);
  TEST_ASSERT_EQUAL(q2.NQUBITS, qregistry.registers[1].numQubits);
  TEST_ASSERT_NOT_NULL(qregistry.registers[1].cpuAmps);
  TEST_ASSERT_EQUAL(q3.NQUBITS, qregistry.registers[2].numQubits);
  TEST_ASSERT_NOT_NULL(qregistry.registers[2].cpuAmps);
  TEST_ASSERT_EQUAL(q1.NQUBITS, qregistry.registers[3].numQubits);
  TEST_ASSERT_NOT_NULL(qregistry.registers[3].cpuAmps);
  TEST_ASSERT_EQUAL_INT(4, get_next_available_qregistry_slot());

  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, device_dealloc_qureg(&q2));
  TEST_ASSERT_EQUAL_size_t(3, qregistry.num_registers);
  TEST_ASSERT_EQUAL_INT(1, get_next_available_qregistry_slot());

  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, device_dealloc_qureg(&q3));
  // Since the 4th slot was last allocated with q1, q1.qregistry_idx = 3
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, device_dealloc_qureg(&q1));
  q1.qregistry_idx = 0;
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, device_dealloc_qureg(&q1));
  TEST_ASSERT_EQUAL_size_t(0, qregistry.num_registers);

  for (size_t i_qr = 0; i_qr < __DEVICE_MAX_NUM_QUREGS__; ++i_qr) {
    TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, device_alloc_qureg(&q1));
  }
  TEST_ASSERT_EQUAL_size_t(__DEVICE_MAX_NUM_QUREGS__, qregistry.num_registers);
  TEST_ASSERT_EQUAL_INT(CQ_ERROR, get_next_available_qregistry_slot());
  TEST_ASSERT_EQUAL_INT(CQ_ERROR, device_alloc_qureg(&q1));

  for (size_t i_qr = 0; i_qr < __DEVICE_MAX_NUM_QUREGS__; ++i_qr) {
    q1.qregistry_idx = i_qr;
    TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, device_dealloc_qureg(&q1));
  }
  TEST_ASSERT_EQUAL_size_t(0, qregistry.num_registers);

  return;
}

void test_clear_qregistry(void) {
  device_alloc_params q1 = {
    .NQUBITS = 1,
    .qregistry_idx = 0,
    .status = CQ_ERROR
  };
  char msg[12];

  for (size_t i = 0; i < __DEVICE_MAX_NUM_QUREGS__; ++i) {
    device_alloc_qureg(&q1);
    sprintf(msg, "idx = %lu", i);
    TEST_ASSERT_FALSE_MESSAGE(qregistry.available[i], msg);
  }
  TEST_ASSERT_EQUAL_size_t(__DEVICE_MAX_NUM_QUREGS__, qregistry.num_registers);

  clear_qregistry();
  TEST_ASSERT_EQUAL_size_t(0, qregistry.num_registers);
  
  for (size_t i = 0; i < __DEVICE_MAX_NUM_QUREGS__; ++i) {
    sprintf(msg, "idx = %lu", i);
    TEST_ASSERT_TRUE_MESSAGE(qregistry.available[i], msg);
  }

  return;
}
