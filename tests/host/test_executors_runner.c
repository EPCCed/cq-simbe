#include "unity.h"
#include "env.h"
#include "kernel_utils.h"
#include "tests/test_qkerns.h"
#include "test_executors.h"

int main ()
{
  UnityBegin("test_executors.c");

  cq_init(0);

  register_qkern(all_site_hadamard);
  register_qkern(zero_init_full_qft);
  register_qkern(equal_superposition_full_qft);
  register_qkern(only_measure_first_site);
  register_qkern(no_measure_qkern);

  RUN_TEST(test_first_run);
  RUN_TEST(test_nmeasure);
  RUN_TEST(test_nshots);
  RUN_TEST(test_bad_inputs);

  cq_finalise(0);

  return UnityEnd();
}