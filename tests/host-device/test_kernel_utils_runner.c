#include "unity.h"
#include "env.h"
#include "test_kernel_utils.h"

int main (void)
{
  UnityBegin("test_kernel_utils.c");

  cq_init(0);

  RUN_TEST(test_register_qkern);
  RUN_TEST(test_find_qkern_pointer);
  RUN_TEST(test_find_qkern_name);

  cq_finalise(0);

  return UnityEnd();
}
