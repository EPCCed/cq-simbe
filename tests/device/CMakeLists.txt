# These tests require using QuEST directly
find_package(QuEST 4.0 REQUIRED)

find_library(MATHS_LIBRARY m REQUIRED)

add_executable(test_device_resource_management
  test_device_resource_management_runner.c
  test_device_resource_management.c
)

target_link_libraries(test_device_resource_management 
  PRIVATE 
  CQ-SIMBE::cq-simbe 
  unity::framework
  QuEST::QuEST
)

add_test(
  NAME test_device_resource_management
  COMMAND ./test_device_resource_management
)

add_executable(test_device_utils
  test_device_utils_runner.c
  test_device_utils.c
)

target_link_libraries(test_device_utils
  PRIVATE
  CQ-SIMBE::cq-simbe
  unity::framework
  QuEST::QuEST
)

add_test(
  NAME test_device_utils
  COMMAND ./test_device_utils
)

add_executable(test_device_control
  test_device_control_runner.c
  test_device_control.c
  ../test_qkerns.c
)

target_link_libraries(test_device_control
  PRIVATE
  CQ-SIMBE::cq-simbe
  unity::framework
  QuEST::QuEST
  ${MATHS_LIBRARY}
)

add_test(
  NAME test_device_control
  COMMAND ./test_device_control
)

add_executable(test_device_ops
  test_device_ops_runner.c
  test_device_ops.c
)

target_compile_definitions(test_device_ops PRIVATE UNITY_INCLUDE_DOUBLE)

target_link_libraries(test_device_ops
  PRIVATE
  CQ-SIMBE::cq-simbe
  unity::framework
  QuEST::QuEST
)

add_test(
  NAME test_device_ops
  COMMAND ./test_device_ops
)