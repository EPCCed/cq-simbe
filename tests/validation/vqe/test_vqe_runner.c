#include "unity.h"
#include "test_vqe.h"

int main (void)
{
  UnityBegin("test_vqe.c");
  cq_init(0);

  RUN_TEST(test_ansatz);

  cq_finalise(0);

  return UnityEnd();
}
