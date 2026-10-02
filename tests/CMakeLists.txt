add_library(cq-simbe-test-support INTERFACE)
target_link_libraries(cq-simbe-test-support INTERFACE
  CQ-SIMBE::cq-simbe unity::framework
)
target_include_directories(cq-simbe-test-support INTERFACE "${PROJECT_SOURCE_DIR}")

# These unit suites inspect in-process simulator/queue state directly.
if(NOT CQ_SIMBE_ENABLE_MPI)
  add_subdirectory(host)
  add_subdirectory(device)
  add_subdirectory(host-device)
  add_subdirectory(validation)
endif()
add_subdirectory(mpi)
if(CQ_SIMBE_BUILD_FORTRAN)
  add_subdirectory(fortran)
endif()

if(CQ_SIMBE_BUILD_EXAMPLES)
  set(_cq_examples sqft aqft qaa)
  if(CQ_SIMBE_BUILD_VQE)
    list(APPEND _cq_examples vqe)
  endif()
  if(CQ_SIMBE_BUILD_FORTRAN)
    list(APPEND _cq_examples zero-qft plus-qft)
  endif()
  foreach(example IN LISTS _cq_examples)
    if(CQ_SIMBE_ENABLE_MPI)
      add_test(NAME example_${example}
        COMMAND ${MPIEXEC_EXECUTABLE} ${MPIEXEC_NUMPROC_FLAG} 2
        ${MPIEXEC_PREFLAGS} $<TARGET_FILE:${example}> ${MPIEXEC_POSTFLAGS})
    else()
      add_test(NAME example_${example} COMMAND ${example})
    endif()
    set_tests_properties(example_${example} PROPERTIES TIMEOUT 120 ENVIRONMENT "OMP_NUM_THREADS=2")
  endforeach()
  foreach(example IN ITEMS mpi_aqft mpi-plus-qft)
    if(TARGET ${example})
      # Each custom-communicator example leaves ranks 0..2 unused by CQ.
      add_test(NAME example_${example}
        COMMAND ${MPIEXEC_EXECUTABLE} ${MPIEXEC_NUMPROC_FLAG} 5
          ${MPIEXEC_PREFLAGS} $<TARGET_FILE:${example}> ${MPIEXEC_POSTFLAGS})
      set_tests_properties(example_${example} PROPERTIES
        TIMEOUT 120 PROCESSORS 5 ENVIRONMENT "OMP_NUM_THREADS=2")
    endif()
  endforeach()
endif()
