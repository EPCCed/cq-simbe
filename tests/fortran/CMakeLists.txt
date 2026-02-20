add_library(fort-test-utils utils.f90)

add_executable(test_fort_host_ops
  test_host_ops.f90
)

target_link_libraries(test_fort_host_ops
  PRIVATE
  CQ-FORT::cq-fort
  fort-test-utils 
)

add_test(
  NAME test_fort_host_ops
  COMMAND ./test_fort_host_ops
)

# TODO: Implement me!
#
#add_executable(test_fort_device_ops
#  test_device_ops.f90
#)
#
#target_link_libraries(test_fort_device_ops
#  PRIVATE
#  CQ-FORT::cq-fort
#  fort-test-utils 
#)
#
#add_test(
#  NAME test_fort_device_ops
#  COMMAND ./test_fort_device_ops
#)
