set_source_files_properties(
  test_device_ops.f90  test_host_ops.f90  test_qasm_gates.f90  utils.f90
  PROPERTIES Fortran_PREPROCESS ON
)

add_executable(test_fort_host_ops
  test_host_ops.f90
  utils.f90
)

target_link_libraries(test_fort_host_ops
  PRIVATE
  cq-simbe
)

add_test(
  NAME test_fort_host_ops
  COMMAND ./test_fort_host_ops
)

add_executable(test_fort_device_ops
  test_device_ops.f90
  utils.f90
)

target_link_libraries(test_fort_device_ops
  PRIVATE
  cq-simbe
)

add_test(
  NAME test_fort_device_ops
  COMMAND ./test_fort_device_ops
)

add_executable(test_fort_qasm_gates
  test_qasm_gates.f90
  utils.f90
)

target_link_libraries(test_fort_qasm_gates
  PRIVATE
  cq-simbe
)

add_test(
  NAME test_fort_qasm_gates
  COMMAND ./test_fort_qasm_gates
)
