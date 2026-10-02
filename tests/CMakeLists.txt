add_library(cq-simbe-test-support INTERFACE)
target_link_libraries(cq-simbe-test-support INTERFACE
  CQ-SIMBE::cq-simbe unity::framework
)
target_include_directories(cq-simbe-test-support INTERFACE "${PROJECT_SOURCE_DIR}")

add_subdirectory(host)
add_subdirectory(device)
add_subdirectory(host-device)
add_subdirectory(validation)
add_subdirectory(mpi)

if(CQ_SIMBE_BUILD_EXAMPLES)
  foreach(example IN ITEMS sqft aqft)
    add_test(NAME example_${example} COMMAND ${example})
    set_tests_properties(example_${example} PROPERTIES TIMEOUT 120)
  endforeach()
endif()
