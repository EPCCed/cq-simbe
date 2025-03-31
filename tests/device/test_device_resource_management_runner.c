#include "unity.h"
#include "env.h"
#include "test_device_resource_management.h"

int main ()
{
  UnityBegin("test_device_resource_management.c");

  cq_init(0);

  RUN_TEST(test_init_qregistry);
  RUN_TEST(test_next_available_slot);
  RUN_TEST(test_device_alloc_and_dealloc_qureg);
  RUN_TEST(test_clear_qregistry);

  cq_finalise(0);

  return UnityEnd();
}