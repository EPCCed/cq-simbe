find_library(math ${MATHS_LIBRARY})

add_executable(test_comms
  test_comms_runner.c
  test_comms.c
)

target_link_libraries(test_comms 
  PRIVATE
  CQ-SIMBE::cq-simbe
  unity::framework
)

add_test(
  NAME test_comms
  COMMAND ./test_comms
)

add_executable(test_kernel_utils
  test_kernel_utils_runner.c
  test_kernel_utils.c
  ../test_qkerns.c
)

target_link_libraries(test_kernel_utils
  PRIVATE
  CQ-SIMBE::cq-simbe
  unity::framework
  ${MATHS_LIBRARY}
)

add_test(
  NAME test_kernel_utils
  COMMAND ./test_kernel_utils
)