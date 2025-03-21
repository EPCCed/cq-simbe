add_executable(test_host_env
  test_host_env_runner.c
  test_host_env.c
)

target_link_libraries(test_host_env PRIVATE CQ-SIMBE::cq-simbe unity::framework)

add_test(
  NAME test_host_env 
  COMMAND ./test_host_env
)