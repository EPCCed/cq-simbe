#include "unity.h"
#include "test_device_utils.h"

int main (void)
{
  UnityBegin("test_device_utils.c");

  RUN_TEST(test_qindex_to_cstate);

  return UnityEnd();
}