cmake_minimum_required(VERSION 4.0)
project(CQSimBEConsumer LANGUAGES C)

option(CONSUMER_ENABLE_CXX "Enable CXX for static dependency packages" OFF)
if(CONSUMER_ENABLE_CXX)
  enable_language(CXX)
endif()

find_package(CQ-SIMBE 0.2 CONFIG REQUIRED)
if(TARGET CQ-SIMBE::fortran)
  message(FATAL_ERROR "The core package unexpectedly loaded the Fortran component")
endif()
add_executable(consumer main.c)
target_link_libraries(consumer PRIVATE CQ-SIMBE::cq-simbe)

# Use the installed file set as the public-header inventory. Each header must
# compile independently, without the producer's source directory or QuEST.
get_target_property(public_headers CQ-SIMBE::cq-simbe HEADER_SET)
if(NOT public_headers)
  message(FATAL_ERROR "The installed target has no public header file set")
endif()
list(LENGTH public_headers header_count)
if(NOT header_count EQUAL 10)
  message(FATAL_ERROR "Expected ten installed public C headers, got ${header_count}")
endif()
foreach(header IN LISTS public_headers)
  get_filename_component(name "${header}" NAME)
  set(source "${CMAKE_CURRENT_BINARY_DIR}/headers/${name}.c")
  file(WRITE "${source}" "#include <${name}>\n")
  list(APPEND header_sources "${source}")
endforeach()
add_library(public_headers OBJECT ${header_sources})
target_link_libraries(public_headers PRIVATE CQ-SIMBE::cq-simbe)

enable_testing()
add_test(NAME installed_consumer COMMAND consumer)
set_tests_properties(installed_consumer PROPERTIES TIMEOUT 60)
get_target_property(core_definitions CQ-SIMBE::cq-simbe INTERFACE_COMPILE_DEFINITIONS)
if("CQ_WITH_MPI_COMMS=1" IN_LIST core_definitions)
  add_test(NAME installed_consumer_mpi
    COMMAND ${MPIEXEC_EXECUTABLE} ${MPIEXEC_NUMPROC_FLAG} 2
      ${MPIEXEC_PREFLAGS} $<TARGET_FILE:consumer> ${MPIEXEC_POSTFLAGS})
  set_tests_properties(installed_consumer_mpi PROPERTIES TIMEOUT 60)
endif()
