#include "unity.h"
#include "test_device_control.h"

int main(void)
{
  UnityBegin("test_device_control.c");

  RUN_TEST(test_initialise_simulator);
  RUN_TEST(test_run_qkernel);
  RUN_TEST(test_finalise_simulator);

  return UnityEnd();
}