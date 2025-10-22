#include "test_vqe.h"
#include "cq.h"
#include "unity.h"

int main (void)
{
  UnityBegin("test_vqe.c");
  cq_init(0);

  RUN_TEST(test_ansatz);

  cq_finalise(0);

  return UnityEnd();
}
