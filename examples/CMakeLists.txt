add_subdirectory(qft)
add_subdirectory(analog)

find_package(NLopt QUIET)

if (${NLopt_FOUND} OR FORCE_BUILD_VQE)
  add_subdirectory(vqe)
else()
  message(STATUS "Could not find NLopt: skipping VQE example. Try FORCE_BUILD_VQE=ON if needed.")
endif()

if(BUILD_FORTRAN_INTERFACE)
  add_subdirectory(fort)
endif()
