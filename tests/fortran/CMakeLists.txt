
add_executable(test_fort_host_ops
  test_host_ops.f90
)

target_link_libraries(test_fort_host_ops PRIVATE CQ-FORT::cq-fort)

add_test(
NAME test_fort_host_ops
COMMAND ./test_fort_host_ops
)
