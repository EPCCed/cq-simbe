#include "unity.h"
#include "env.h"
#include "test_host_env.h"

int main ()
{
  UnityBegin("test_host_env.c");

  RUN_TEST(test_initial_environment_flags);
  RUN_TEST(test_init);
  RUN_TEST(test_double_init);
  RUN_TEST(test_finalise);
  RUN_TEST(test_double_finalise);
  RUN_TEST(test_reinitialise);

  return UnityEnd();
}