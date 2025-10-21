add_subdirectory(qft)
add_subdirectory(analog)

find_package(NLopt QUIET)

if (${NLopt_FOUND})
  add_subdirectory(vqe)
else()
  message(STATUS "Could not find NLopt: skipping VQE example.")
endif()
