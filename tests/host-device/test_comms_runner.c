#include "test_comms.h"
#include "unity.h"

int main (void)
{
  UnityBegin("test_comms.c");

  RUN_TEST(test_initialise_device);
  RUN_TEST(test_host_send_and_wait_ctrl_op);
  RUN_TEST(test_op_queue);
  RUN_TEST(test_finalise_device);

  return UnityEnd();
}