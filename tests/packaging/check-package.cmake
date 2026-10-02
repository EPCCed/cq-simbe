cmake_minimum_required(VERSION 4.0)

if(NOT DEFINED SOURCE_DIR)
  get_filename_component(SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)
endif()
if(NOT DEFINED BINARY_DIR)
  set(BINARY_DIR "${SOURCE_DIR}/out/package-check")
endif()
if(NOT DEFINED BUILD_JOBS)
  set(BUILD_JOBS 2)
endif()
get_filename_component(_cmake_bin "${CMAKE_COMMAND}" DIRECTORY)
set(_ctest "${_cmake_bin}/ctest")

function(run)
  execute_process(COMMAND ${ARGV} RESULT_VARIABLE status
    OUTPUT_VARIABLE output ERROR_VARIABLE error)
  if(NOT status STREQUAL "0")
    string(JOIN " " command ${ARGV})
    message(FATAL_ERROR "Command failed (${status}): ${command}\n${output}\n${error}")
  endif()
endfunction()

set(producer_flags)
foreach(option IN ITEMS CQ_SIMBE_BUILD_FORTRAN CQ_SIMBE_ENABLE_MPI)
  if(DEFINED ${option})
    list(APPEND producer_flags "-D${option}=${${option}}")
  endif()
endforeach()

foreach(kind IN ITEMS shared static)
  set(work "${BINARY_DIR}/${kind}")
  file(REMOVE_RECURSE "${work}")
  set(shared OFF)
  set(enable_cxx ON)
  if(kind STREQUAL "shared")
    set(shared ON)
    set(enable_cxx OFF)
  endif()
  message(STATUS "Checking ${kind} installation and downstream consumers")
  run("${CMAKE_COMMAND}" -S "${SOURCE_DIR}" -B "${work}/producer" -G Ninja
    -DCMAKE_BUILD_TYPE=Release "-DBUILD_SHARED_LIBS=${shared}"
    -DCQ_SIMBE_BUILD_TESTS=OFF -DCQ_SIMBE_BUILD_EXAMPLES=OFF
    -DCQ_SIMBE_ENABLE_INSTALL=ON -DCMAKE_DISABLE_FIND_PACKAGE_unity=TRUE
    "-DCMAKE_INSTALL_PREFIX=${work}/prefix" -DCMAKE_INSTALL_LIBDIR=lib ${producer_flags})
  run("${CMAKE_COMMAND}" --build "${work}/producer" --parallel "${BUILD_JOBS}")
  run("${CMAKE_COMMAND}" --install "${work}/producer")

  file(GLOB_RECURSE metadata "${work}/prefix/lib/cmake/CQ-SIMBE/*.cmake")
  if(NOT metadata)
    message(FATAL_ERROR "No installed CMake package")
  endif()
  foreach(config IN LISTS metadata)
    file(READ "${config}" contents)
    foreach(forbidden IN ITEMS "${SOURCE_DIR}" "${work}/producer" "${work}/prefix")
      string(FIND "${contents}" "${forbidden}" position)
      if(NOT position EQUAL -1)
        message(FATAL_ERROR "Non-relocatable path ${forbidden} in ${config}")
      endif()
    endforeach()
  endforeach()
  if(EXISTS "${work}/prefix/include/src" OR EXISTS "${work}/prefix/include/mpi_runtime.h")
    message(FATAL_ERROR "Internal headers were installed")
  endif()

  # Only static consumers need QuEST development metadata. Explicitly forbid
  # discovering it for the C-only shared consumer, including in a Nix shell.
  set(consumer_flags "-DCONSUMER_ENABLE_CXX=${enable_cxx}")
  if(shared)
    list(APPEND consumer_flags -DCMAKE_DISABLE_FIND_PACKAGE_QuEST=TRUE)
    foreach(case IN ITEMS old-version fortran-disabled fortran-mismatch)
      if(case STREQUAL "fortran-mismatch" AND NOT CQ_SIMBE_BUILD_FORTRAN)
        continue()
      endif()
      execute_process(COMMAND "${CMAKE_COMMAND}"
        -S "${SOURCE_DIR}/tests/packaging/rejection"
        -B "${work}/reject-${case}" -G Ninja "-DCHECK_CASE=${case}"
        "-DCQ-SIMBE_DIR=${work}/prefix/lib/cmake/CQ-SIMBE"
        -DCMAKE_FIND_USE_PACKAGE_REGISTRY=OFF
        RESULT_VARIABLE status OUTPUT_VARIABLE output ERROR_VARIABLE error)
      if(status STREQUAL "0")
        message(FATAL_ERROR "Package unexpectedly accepted ${case}")
      endif()
      if(case STREQUAL "fortran-mismatch" AND NOT "${error}" MATCHES "Fortran modules require")
        message(FATAL_ERROR "Compiler mismatch did not report its cause: ${error}")
      endif()
    endforeach()
  endif()
  foreach(location IN ITEMS prefix relocated)
    if(location STREQUAL "relocated")
      file(RENAME "${work}/prefix" "${work}/relocated")
    endif()
    run("${CMAKE_COMMAND}" -S "${SOURCE_DIR}/tests/packaging/consumer"
      -B "${work}/consumer-${location}" -G Ninja
      "-DCQ-SIMBE_DIR=${work}/${location}/lib/cmake/CQ-SIMBE"
      -DCMAKE_FIND_USE_PACKAGE_REGISTRY=OFF ${consumer_flags})
    run("${CMAKE_COMMAND}" --build "${work}/consumer-${location}" --parallel "${BUILD_JOBS}")
    run("${_ctest}" --test-dir "${work}/consumer-${location}" --output-on-failure)
    if(CQ_SIMBE_BUILD_FORTRAN)
      run("${CMAKE_COMMAND}" -S "${SOURCE_DIR}/tests/packaging/fortran"
        -B "${work}/fortran-${location}" -G Ninja
        "-DCQ-SIMBE_DIR=${work}/${location}/lib/cmake/CQ-SIMBE"
        -DCMAKE_FIND_USE_PACKAGE_REGISTRY=OFF ${consumer_flags})
      run("${CMAKE_COMMAND}" --build "${work}/fortran-${location}" --parallel "${BUILD_JOBS}")
      run("${_ctest}" --test-dir "${work}/fortran-${location}" --output-on-failure)
    endif()
  endforeach()
endforeach()

message(STATUS "Checking add_subdirectory defaults without Unity")
set(work "${BINARY_DIR}/embedded")
file(REMOVE_RECURSE "${work}")
run("${CMAKE_COMMAND}" -S "${SOURCE_DIR}/tests/packaging/embedded" -B "${work}" -G Ninja
  "-DCQ_SIMBE_SOURCE_DIR=${SOURCE_DIR}")
run("${CMAKE_COMMAND}" --build "${work}" --parallel "${BUILD_JOBS}")
run("${_ctest}" --test-dir "${work}" --output-on-failure)
message(STATUS "Shared/static install, relocation, header, component, and embedding checks passed")
