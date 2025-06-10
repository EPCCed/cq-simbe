#include "env.h"
#include "unity.h"
#include "test_device_ops.h"

int main (void)
{
  UnityBegin("test_device_ops.c");

  cq_init(0);

  RUN_TEST(test_qureg_setters);
  RUN_TEST(test_qabort);
  RUN_TEST(test_qureg_measure);

  cq_finalise(0);

  return UnityEnd();
}