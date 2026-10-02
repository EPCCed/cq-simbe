add_executable(cq_test_lifecycle test_lifecycle.c)
target_link_libraries(cq_test_lifecycle PRIVATE CQ-SIMBE::cq-simbe QuEST::QuEST)

# Isolated executable uses the real host and queue sources with fault injection.
add_executable(cq_test_startup_failure test_startup_failure.c)
target_include_directories(cq_test_startup_failure PRIVATE
  "${PROJECT_SOURCE_DIR}" "${PROJECT_SOURCE_DIR}/include")
target_link_libraries(cq_test_startup_failure PRIVATE Threads::Threads QuEST::QuEST)
foreach(failure RANGE 0 5)
  add_test(NAME cq_startup_failure_${failure}
    COMMAND cq_test_startup_failure ${failure})
  set_tests_properties(cq_startup_failure_${failure} PROPERTIES TIMEOUT 15)
endforeach()

if(CQ_SIMBE_HAS_MPI)
  target_link_libraries(cq_test_lifecycle PRIVATE MPI::MPI_C)
  add_executable(cq_test_mpi_runtime test_mpi_runtime.c)
  target_include_directories(cq_test_mpi_runtime PRIVATE
    "${PROJECT_SOURCE_DIR}" "${PROJECT_SOURCE_DIR}/include")
  target_link_libraries(cq_test_mpi_runtime PRIVATE
    MPI::MPI_C Threads::Threads QuEST::QuEST)
  add_executable(cq_test_rank_startup_failure test_rank_startup_failure.c)
  target_include_directories(cq_test_rank_startup_failure PRIVATE
    "${PROJECT_SOURCE_DIR}" "${PROJECT_SOURCE_DIR}/include")
  target_link_libraries(cq_test_rank_startup_failure PRIVATE
    MPI::MPI_C Threads::Threads QuEST::QuEST)
  foreach(mode owned borrowed owned-foreign-quest)
    add_test(NAME cq_mpi_rank_startup_failure_${mode}
      COMMAND ${MPIEXEC_EXECUTABLE} ${MPIEXEC_NUMPROC_FLAG} 2
              ${MPIEXEC_PREFLAGS} $<TARGET_FILE:cq_test_rank_startup_failure>
              ${MPIEXEC_POSTFLAGS} ${mode})
    set_tests_properties(cq_mpi_rank_startup_failure_${mode} PROPERTIES
      TIMEOUT 60 PROCESSORS 2 SKIP_RETURN_CODE 77)
  endforeach()
  foreach(mode owned owned-low owned-peer owned-nonpower owned-local-unready
               borrowed borrowed-low borrowed-local-unready
               borrowed-peer borrowed-late-peer borrowed-nonpower finalized init-error)
    add_test(NAME cq_mpi_runtime_${mode} COMMAND cq_test_mpi_runtime ${mode})
    set_tests_properties(cq_mpi_runtime_${mode} PROPERTIES TIMEOUT 15)
  endforeach()
  foreach(ranks 1 2)
    foreach(mode owned borrowed insufficient finalized foreign-quest)
      add_test(NAME cq_mpi_${mode}_${ranks}
        COMMAND ${MPIEXEC_EXECUTABLE} ${MPIEXEC_NUMPROC_FLAG} ${ranks}
                ${MPIEXEC_PREFLAGS} $<TARGET_FILE:cq_test_lifecycle>
                ${MPIEXEC_POSTFLAGS} ${mode})
      set_tests_properties(cq_mpi_${mode}_${ranks} PROPERTIES
        TIMEOUT 60 PROCESSORS ${ranks})
    endforeach()
  endforeach()
  foreach(mode nonpower owned-nonpower)
    add_test(NAME cq_mpi_${mode}_3
      COMMAND ${MPIEXEC_EXECUTABLE} ${MPIEXEC_NUMPROC_FLAG} 3
              ${MPIEXEC_PREFLAGS} $<TARGET_FILE:cq_test_lifecycle>
              ${MPIEXEC_POSTFLAGS} ${mode})
    set_tests_properties(cq_mpi_${mode}_3 PROPERTIES TIMEOUT 60 PROCESSORS 3)
  endforeach()
else()
  foreach(mode owned foreign-quest)
    add_test(NAME cq_serial_${mode} COMMAND cq_test_lifecycle ${mode})
    set_tests_properties(cq_serial_${mode} PROPERTIES TIMEOUT 30)
  endforeach()
endif()
