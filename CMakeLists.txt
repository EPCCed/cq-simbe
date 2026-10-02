cmake_minimum_required(VERSION 4.0)

project(CQ-SIMBE
  VERSION 0.2.0
  DESCRIPTION "CQ compliant simulated backend."
  LANGUAGES C CXX
)

option(BUILD_SHARED_LIBS "Build shared libraries" ON)
function(cq_simbe_option modern legacy description default)
  if(DEFINED ${legacy} AND NOT DEFINED ${modern})
    message(DEPRECATION "${legacy} is deprecated; use ${modern}")
    set(default "${${legacy}}")
  endif()
  option(${modern} "${description}" "${default}")
endfunction()
cq_simbe_option(CQ_SIMBE_BUILD_TESTS ENABLE_TESTING
  "Build cq-simbe tests (also requires BUILD_TESTING)" "${PROJECT_IS_TOP_LEVEL}")
cq_simbe_option(CQ_SIMBE_BUILD_EXAMPLES BUILD_EXAMPLES
  "Build cq-simbe examples" "${PROJECT_IS_TOP_LEVEL}")
cq_simbe_option(CQ_SIMBE_BUILD_FORTRAN BUILD_FORTRAN_INTERFACE
  "Build optional Fortran bindings" OFF)
cq_simbe_option(CQ_SIMBE_BUILD_VQE FORCE_BUILD_VQE
  "Build the VQE example (requires NLopt)" OFF)
option(CQ_SIMBE_ENABLE_INSTALL "Generate cq-simbe installation rules" "${PROJECT_IS_TOP_LEVEL}")

set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)
find_package(QuEST 4.3 CONFIG REQUIRED)
include(cmake/QuESTFeatures.cmake)
cq_simbe_option(CQ_SIMBE_ENABLE_MPI BUILD_WITH_MPI
  "Use MPI host/device transport" "${CQ_SIMBE_HAS_MPI}")
if(DEFINED QUEST_BUILT_WITH_MPI)
  message(DEPRECATION "QUEST_BUILT_WITH_MPI is ignored; QuEST capabilities are detected from its installed configuration")
endif()

add_library(cq-simbe)
add_library(CQ-SIMBE::cq-simbe ALIAS cq-simbe)
set_target_properties(cq-simbe PROPERTIES VERSION "${PROJECT_VERSION}" SOVERSION 0.2)
if(CQ_SIMBE_ENABLE_MPI)
  target_compile_definitions(cq-simbe PUBLIC CQ_WITH_MPI_COMMS=1)
else()
  target_compile_definitions(cq-simbe PUBLIC CQ_WITH_MPI_COMMS=0)
endif()
target_compile_features(cq-simbe PUBLIC c_std_99)
target_compile_options(cq-simbe PRIVATE
  "$<$<C_COMPILER_ID:GNU,Clang,AppleClang>:-Wall>"
  "$<$<C_COMPILER_ID:MSVC>:/W4>"
)
target_sources(cq-simbe
  PUBLIC FILE_SET HEADERS BASE_DIRS include FILES
    include/cq.h
    include/analog.h
    include/comms_utils.h
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
      src/host-device/comms/comms_core.h
      src/host-device/comms/comms_mpi.h
      src/device/analog/analog_datatypes.h
      src/device/analog/analog_device.h
      src/device/analog/analog_qreg.h
      src/device/analog/channel.h
      src/device/analog/hamil.h
      src/device/analog/pulse.h
      src/device/analog/simulator.h
      src/device/analog/waveforms.h
)
target_link_libraries(cq-simbe PUBLIC Threads::Threads PRIVATE QuEST::QuEST)
if(CQ_SIMBE_ENABLE_MPI OR CQ_SIMBE_HAS_MPI)
  find_package(MPI REQUIRED COMPONENTS C)
  if(CQ_SIMBE_ENABLE_MPI)
    target_link_libraries(cq-simbe PUBLIC MPI::MPI_C)
  else()
    target_link_libraries(cq-simbe PRIVATE MPI::MPI_C)
  endif()
endif()
if(UNIX)
  # Keep the system library name relocatable instead of exporting a host path.
  set(CQ_SIMBE_MATH_LIBRARY m)
  target_link_libraries(cq-simbe PRIVATE ${CQ_SIMBE_MATH_LIBRARY})
endif()

add_subdirectory(src)
if(CQ_SIMBE_BUILD_FORTRAN)
  include(cmake/Fortran.cmake)
endif()
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
