# These tests require using QuEST directly
find_package(QuEST 4.0 REQUIRED)

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