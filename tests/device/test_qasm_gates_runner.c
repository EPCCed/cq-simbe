#include "unity.h"
#include "env.h"
#include "test_qasm_gates.h"

int main(void)
{
  UnityBegin("test_qasm_gates.c");

  cq_init(0);

  RUN_TEST(test_unitary);
  RUN_TEST(test_gphase);
  RUN_TEST(test_phase);
  RUN_TEST(test_paulix);
  RUN_TEST(test_pauliy);
  RUN_TEST(test_pauliz);
  RUN_TEST(test_hadamard);
  RUN_TEST(test_cphase);
  RUN_TEST(test_swap);

  cq_finalise(0);

  return UnityEnd();
}