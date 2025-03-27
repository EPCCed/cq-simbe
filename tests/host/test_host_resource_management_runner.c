#include "unity.h"
#include "env.h"
#include "test_host_resource_management.h"

int main ()
{
  UnityBegin("test_host_resource_management.c");

  cq_init(0);

  RUN_TEST(test_first_alloc_qureg);
  RUN_TEST(test_first_free_qureg);
  RUN_TEST(test_alloc_and_free_qureg);
  RUN_TEST(test_realloc_qureg);
  RUN_TEST(test_double_free_qureg);

  cq_finalise(0);

  return UnityEnd();
}