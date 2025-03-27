# Need maths library
find_library(MATHS_LIBRARY m REQUIRED)

add_executable(test_host_env
  test_host_env_runner.c
  test_host_env.c
)

target_link_libraries(test_host_env PRIVATE CQ-SIMBE::cq-simbe unity::framework)

add_test(
  NAME test_host_env 
  COMMAND ./test_host_env
)

add_executable(test_host_resource_management
  test_host_resource_management_runner.c
  test_host_resource_management.c
)

target_link_libraries(test_host_resource_management 
  PRIVATE 
  CQ-SIMBE::cq-simbe 
  unity::framework
)

add_test(
  NAME test_host_resource_management
  COMMAND ./test_host_resource_management
)

add_executable(test_executors
  test_executors_runner.c
  test_executors.c
  test_qkerns.c
)

target_link_libraries(test_executors 
  PRIVATE
  CQ-SIMBE::cq-simbe
  unity::framework
  ${MATHS_LIBRARY}
)

add_test(
  NAME test_executors
  COMMAND ./test_executors
)