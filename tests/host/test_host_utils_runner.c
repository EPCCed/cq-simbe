#include "unity.h"
#include "test_host_utils.h"

int main (void)
{
  UnityBegin("test_host_utils.c");

  RUN_TEST(test_init_creg);

  return UnityEnd();
}