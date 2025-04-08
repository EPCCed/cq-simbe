#include "unity.h"
#include "include/env.h"
#include "test_qft.h"

int main (void)
{
  UnityBegin("test_qft.c");

  cq_init(0);

  RUN_TEST(test_zero_init_qft);
  RUN_TEST(test_plus_init_qft);

  cq_finalise(0);

  return UnityEnd();
}