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
  RUN_TEST(test_sqrtz);
  RUN_TEST(test_sqrtzhc);
  RUN_TEST(test_sqrts);
  RUN_TEST(test_sqrtshc);
  RUN_TEST(test_sqrtx);
  RUN_TEST(test_rotx);
  RUN_TEST(test_roty);
  RUN_TEST(test_rotz);
  RUN_TEST(test_cpaulix);
  RUN_TEST(test_cpauliy);
  RUN_TEST(test_cpauliz);
  RUN_TEST(test_cphase);
  RUN_TEST(test_crotx);
  RUN_TEST(test_croty);
  RUN_TEST(test_crotz);
  RUN_TEST(test_chadamard);
  RUN_TEST(test_cunitary);
  RUN_TEST(test_swap);
  RUN_TEST(test_ccpaulix);
  RUN_TEST(test_cswap);

  cq_finalise(0);

  return UnityEnd();
}