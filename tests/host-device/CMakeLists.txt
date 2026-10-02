add_executable(test_comms
  test_comms_runner.c
  test_comms.c
)

target_link_libraries(test_comms 
  PRIVATE
  cq-simbe-test-support
)

add_test(
  NAME test_comms
  COMMAND test_comms
)

add_executable(test_kernel_utils
  test_kernel_utils_runner.c
  test_kernel_utils.c
  ../test_qkerns.c
)

target_link_libraries(test_kernel_utils
  PRIVATE
  cq-simbe-test-support
  ${CQ_SIMBE_MATH_LIBRARY}
)

add_test(
  NAME test_kernel_utils
  COMMAND test_kernel_utils
)
