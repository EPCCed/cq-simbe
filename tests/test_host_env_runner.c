#include "unity.h"
#include "env.h"

extern void test_initialisation_return_zero(void);

int main ()
{
  UnityBegin("test_host_env.c");

  RUN_TEST(test_initialisation_return_zero);

  return UnityEnd();
}