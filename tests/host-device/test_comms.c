#include <stdbool.h>
#include "unity.h"
#include "datatypes.h"
#include "src/host/opcodes.h"
#include "src/host-device/comms.h"
#include "test_comms.h"

// TODO should put some more sophisticated testing of the thread management in

void setUp(void) {
  return;
}

void tearDown(void) {
  return;
}

void test_initialise_device(void) {
  const unsigned int VERBOSITY = 0;
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, initialise_device(VERBOSITY));
  TEST_ASSERT(dev_ctrl.run_device);
  TEST_ASSERT_FALSE(dev_ctrl.device_busy);
  TEST_ASSERT_EQUAL_size_t(0, dev_ctrl.num_ops);
  TEST_ASSERT_EACH_EQUAL_INT(CQ_CTRL_IDLE, dev_ctrl.op_buffer, __CQ_DEVICE_QUEUE_SIZE__);
  TEST_ASSERT_EACH_EQUAL_PTR(NULL, dev_ctrl.op_params_buffer, __CQ_DEVICE_QUEUE_SIZE__);

  return;
}

void test_host_send_and_wait_ctrl_op(void) {
  bool test_flag = false;

  TEST_ASSERT_FALSE(test_flag);
  host_send_ctrl_op(CQ_CTRL_TEST, &test_flag);
  TEST_ASSERT_FALSE(test_flag); // may be sensitive to timing

  host_wait_all_ops();
  TEST_ASSERT(test_flag);
  TEST_ASSERT_EQUAL_size_t(0, dev_ctrl.num_ops);
  TEST_ASSERT_FALSE(dev_ctrl.device_busy);
  TEST_ASSERT_EACH_EQUAL_INT(CQ_CTRL_IDLE, dev_ctrl.op_buffer, __CQ_DEVICE_QUEUE_SIZE__);
  TEST_ASSERT_EACH_EQUAL_PTR(NULL, dev_ctrl.op_params_buffer, __CQ_DEVICE_QUEUE_SIZE__);

  return;
}

void test_finalise_device(void) {
  const unsigned int VERBOSITY = 0;
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, finalise_device(VERBOSITY));
  TEST_ASSERT_FALSE(dev_ctrl.run_device);
  TEST_ASSERT_FALSE(dev_ctrl.device_busy);
  TEST_ASSERT_EQUAL_size_t(0, dev_ctrl.num_ops);
  TEST_ASSERT_EACH_EQUAL_INT(CQ_CTRL_IDLE, dev_ctrl.op_buffer, __CQ_DEVICE_QUEUE_SIZE__);
  TEST_ASSERT_EACH_EQUAL_PTR(NULL, dev_ctrl.op_params_buffer, __CQ_DEVICE_QUEUE_SIZE__);

  return;
}