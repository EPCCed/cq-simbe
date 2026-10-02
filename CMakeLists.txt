cmake_minimum_required(VERSION 4.0)

project(CQ-SIMBE
  VERSION 0.1.0
  DESCRIPTION "CQ compliant simulated backend."
  LANGUAGES C CXX
)

option(BUILD_SHARED_LIBS "Build shared libraries" ON)
set(_cq_simbe_tests_default "${PROJECT_IS_TOP_LEVEL}")
if(DEFINED ENABLE_TESTING AND NOT DEFINED CQ_SIMBE_BUILD_TESTS)
  message(DEPRECATION "ENABLE_TESTING is deprecated; use CQ_SIMBE_BUILD_TESTS")
  set(_cq_simbe_tests_default "${ENABLE_TESTING}")
endif()
option(CQ_SIMBE_BUILD_TESTS "Build cq-simbe tests (also requires BUILD_TESTING)" "${_cq_simbe_tests_default}")
option(CQ_SIMBE_BUILD_EXAMPLES "Build cq-simbe examples" "${PROJECT_IS_TOP_LEVEL}")
option(CQ_SIMBE_ENABLE_INSTALL "Generate cq-simbe installation rules" "${PROJECT_IS_TOP_LEVEL}")
unset(_cq_simbe_tests_default)

set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)
find_package(QuEST 4.3 CONFIG REQUIRED)
include(cmake/QuESTFeatures.cmake)

add_library(cq-simbe)
add_library(CQ-SIMBE::cq-simbe ALIAS cq-simbe)
set_target_properties(cq-simbe PROPERTIES VERSION "${PROJECT_VERSION}" SOVERSION 0.1)
target_compile_features(cq-simbe PUBLIC c_std_99)
target_compile_options(cq-simbe PRIVATE
  "$<$<C_COMPILER_ID:GNU,Clang,AppleClang>:-Wall>"
  "$<$<C_COMPILER_ID:MSVC>:/W4>"
)
target_sources(cq-simbe
  PUBLIC FILE_SET HEADERS BASE_DIRS include FILES
    include/cq.h
    include/datatypes.h
    include/device_ops.h
    include/env.h
    include/host_ops.h
    include/kernel_utils.h
    include/qasm_gates.h
    include/utils.h
  PRIVATE FILE_SET internal_headers TYPE HEADERS
    BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}"
    FILES
      src/device/control.h
      src/device/device_utils.h
      src/device/mpi_runtime.h
      src/device/resources.h
      src/host/opcodes.h
      src/host-device/comms.h
)
target_link_libraries(cq-simbe PUBLIC Threads::Threads PRIVATE QuEST::QuEST)
if(CQ_SIMBE_HAS_MPI)
  find_package(MPI REQUIRED COMPONENTS C)
  target_link_libraries(cq-simbe PRIVATE MPI::MPI_C)
endif()
if(UNIX)
  # Keep the system library name relocatable instead of exporting a host path.
  set(CQ_SIMBE_MATH_LIBRARY m)
  target_link_libraries(cq-simbe PRIVATE ${CQ_SIMBE_MATH_LIBRARY})
endif()

add_subdirectory(src)
if(CQ_SIMBE_BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()
if(CQ_SIMBE_BUILD_TESTS)
  include(CTest)
  if(BUILD_TESTING)
    find_package(unity CONFIG REQUIRED)
    add_subdirectory(tests)
  endif()
endif()
if(CQ_SIMBE_ENABLE_INSTALL)
  include(cmake/Install.cmake)
endif()
