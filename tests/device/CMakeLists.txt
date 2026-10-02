# These tests require using QuEST directly


add_executable(test_device_resource_management
  test_device_resource_management_runner.c
  test_device_resource_management.c
)

target_link_libraries(test_device_resource_management
  PRIVATE
  cq-simbe-test-support
  QuEST::QuEST
)

add_test(
  NAME test_device_resource_management
  COMMAND test_device_resource_management
)

add_executable(test_device_utils
  test_device_utils_runner.c
  test_device_utils.c
)

target_link_libraries(test_device_utils
  PRIVATE
  cq-simbe-test-support
  QuEST::QuEST
)

add_test(
  NAME test_device_utils
  COMMAND test_device_utils
)

add_executable(test_device_control
  test_device_control_runner.c
  test_device_control.c
  ../test_qkerns.c
)

target_link_libraries(test_device_control
  PRIVATE
  cq-simbe-test-support
  QuEST::QuEST
  ${CQ_SIMBE_MATH_LIBRARY}
)

add_test(
  NAME test_device_control
  COMMAND test_device_control
)

add_executable(test_device_ops
  test_device_ops_runner.c
  test_device_ops.c
)


target_link_libraries(test_device_ops
  PRIVATE
  cq-simbe-test-support
  QuEST::QuEST
)

add_test(
  NAME test_device_ops
  COMMAND test_device_ops
)

add_executable(test_qasm_gates
  test_qasm_gates_runner.c
  test_qasm_gates.c
)

target_link_libraries(test_qasm_gates
  PRIVATE
  cq-simbe-test-support
  QuEST::QuEST
  ${CQ_SIMBE_MATH_LIBRARY}
)


add_test(
  NAME test_qasm_gates
  COMMAND test_qasm_gates
)

add_subdirectory(analog)
