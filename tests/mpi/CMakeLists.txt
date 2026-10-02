# Public offload regressions link the installed API surface; no Unity fixtures.
if(CQ_SIMBE_ENABLE_MPI)
  add_executable(cq_test_public_mpi test_public_mpi.c)
  target_link_libraries(cq_test_public_mpi PRIVATE CQ-SIMBE::cq-simbe QuEST::QuEST MPI::MPI_C)
  function(cq_offload_test mode ranks)
    add_test(NAME cq_offload_${mode}_${ranks}
      COMMAND ${MPIEXEC_EXECUTABLE} ${MPIEXEC_NUMPROC_FLAG} ${ranks}
        ${MPIEXEC_PREFLAGS} $<TARGET_FILE:cq_test_public_mpi>
        ${MPIEXEC_POSTFLAGS} ${mode})
    set_tests_properties(cq_offload_${mode}_${ranks} PROPERTIES
      TIMEOUT 60 PROCESSORS ${ranks} SKIP_RETURN_CODE 77
      ENVIRONMENT "OMP_NUM_THREADS=2")
  endfunction()
  add_executable(cq_test_offload_startup_failure test_offload_startup_failure.c)
  target_include_directories(cq_test_offload_startup_failure PRIVATE
    "${PROJECT_SOURCE_DIR}" "${PROJECT_SOURCE_DIR}/include")
  target_compile_definitions(cq_test_offload_startup_failure PRIVATE CQ_WITH_MPI_COMMS=1)
  target_link_libraries(cq_test_offload_startup_failure PRIVATE MPI::MPI_C Threads::Threads QuEST::QuEST)
  set(_cq_startup_ranks 2)
  if(CQ_SIMBE_HAS_MPI)
    set(_cq_startup_ranks 3)
  endif()
  foreach(ownership owned borrowed)
    foreach(stage worker listener)
      add_test(NAME cq_offload_startup_${ownership}_${stage}
        COMMAND ${MPIEXEC_EXECUTABLE} ${MPIEXEC_NUMPROC_FLAG} ${_cq_startup_ranks} ${MPIEXEC_PREFLAGS}
          $<TARGET_FILE:cq_test_offload_startup_failure> ${MPIEXEC_POSTFLAGS} ${ownership} ${stage})
      set_tests_properties(cq_offload_startup_${ownership}_${stage} PROPERTIES
        TIMEOUT 60 PROCESSORS ${_cq_startup_ranks} SKIP_RETURN_CODE 77 ENVIRONMENT "OMP_NUM_THREADS=2")
    endforeach()
  endforeach()
  set(_cq_valid_ranks 1 2)
  if(CQ_SIMBE_HAS_MPI)
    list(APPEND _cq_valid_ranks 3 5)
  endif()
  foreach(ranks IN LISTS _cq_valid_ranks)
    foreach(mode owned borrowed custom pending-finalize pending-error)
      cq_offload_test(${mode} ${ranks})
    endforeach()
  endforeach()
  foreach(mode owned-invalid borrowed-invalid custom-invalid)
    cq_offload_test(${mode} 4)
    if(NOT CQ_SIMBE_HAS_MPI)
      cq_offload_test(${mode} 3)
      cq_offload_test(${mode} 5)
    endif()
  endforeach()
  foreach(mode weak-thread finalized custom-null owned-asymmetric borrowed-asymmetric
               delayed-registration missing-registration mismatched-registration missing-worker capacity)
    cq_offload_test(${mode} 2)
  endforeach()
  cq_offload_test(split-pairs 5)
  cq_offload_test(intercomm 4)
  if(CQ_SIMBE_HAS_MPI)
    foreach(ranks 3 5)
      cq_offload_test(distributed-halt ${ranks})
      cq_offload_test(distributed-early ${ranks})
      cq_offload_test(distributed-error ${ranks})
      cq_offload_test(capacity ${ranks})
      cq_offload_test(delayed-registration ${ranks})
      cq_offload_test(missing-registration ${ranks})
      cq_offload_test(missing-worker ${ranks})
      cq_offload_test(mismatched-registration ${ranks})
    endforeach()
    cq_offload_test(split-unused 5)
    cq_offload_test(split-groups 7)
  endif()
else()
  # The original deterministic failure fixtures exercise the thread transport.
  add_executable(cq_test_lifecycle test_lifecycle.c)
  target_link_libraries(cq_test_lifecycle PRIVATE CQ-SIMBE::cq-simbe QuEST::QuEST)
  add_executable(cq_test_startup_failure test_startup_failure.c)
  target_include_directories(cq_test_startup_failure PRIVATE
    "${PROJECT_SOURCE_DIR}" "${PROJECT_SOURCE_DIR}/include")
  target_compile_definitions(cq_test_startup_failure PRIVATE CQ_WITH_MPI_COMMS=0)
  target_link_libraries(cq_test_startup_failure PRIVATE Threads::Threads QuEST::QuEST)
  foreach(failure RANGE 0 5)
    add_test(NAME cq_startup_failure_${failure} COMMAND cq_test_startup_failure ${failure})
    set_tests_properties(cq_startup_failure_${failure} PROPERTIES TIMEOUT 15)
  endforeach()
  if(CQ_SIMBE_HAS_MPI)
    target_link_libraries(cq_test_lifecycle PRIVATE MPI::MPI_C)
    foreach(fixture mpi_runtime rank_startup_failure)
      add_executable(cq_test_${fixture} test_${fixture}.c)
      target_include_directories(cq_test_${fixture} PRIVATE
        "${PROJECT_SOURCE_DIR}" "${PROJECT_SOURCE_DIR}/include")
      target_compile_definitions(cq_test_${fixture} PRIVATE CQ_WITH_MPI_COMMS=0)
      target_link_libraries(cq_test_${fixture} PRIVATE MPI::MPI_C Threads::Threads QuEST::QuEST)
    endforeach()
    foreach(mode owned borrowed owned-foreign-quest)
      add_test(NAME cq_mpi_rank_startup_failure_${mode}
        COMMAND ${MPIEXEC_EXECUTABLE} ${MPIEXEC_NUMPROC_FLAG} 2 ${MPIEXEC_PREFLAGS}
          $<TARGET_FILE:cq_test_rank_startup_failure> ${MPIEXEC_POSTFLAGS} ${mode})
      set_tests_properties(cq_mpi_rank_startup_failure_${mode} PROPERTIES
        TIMEOUT 60 PROCESSORS 2 SKIP_RETURN_CODE 77 ENVIRONMENT "OMP_NUM_THREADS=2")
    endforeach()
    foreach(mode owned owned-low owned-peer owned-nonpower owned-local-unready
                 borrowed borrowed-low borrowed-local-unready borrowed-peer
                 borrowed-late-peer borrowed-nonpower finalized init-error)
      add_test(NAME cq_mpi_runtime_${mode} COMMAND cq_test_mpi_runtime ${mode})
      set_tests_properties(cq_mpi_runtime_${mode} PROPERTIES TIMEOUT 15)
    endforeach()
    foreach(ranks 1 2)
      foreach(mode owned borrowed insufficient finalized foreign-quest)
        add_test(NAME cq_mpi_${mode}_${ranks}
          COMMAND ${MPIEXEC_EXECUTABLE} ${MPIEXEC_NUMPROC_FLAG} ${ranks} ${MPIEXEC_PREFLAGS}
            $<TARGET_FILE:cq_test_lifecycle> ${MPIEXEC_POSTFLAGS} ${mode})
        set_tests_properties(cq_mpi_${mode}_${ranks} PROPERTIES
          TIMEOUT 60 PROCESSORS ${ranks} ENVIRONMENT "OMP_NUM_THREADS=2")
      endforeach()
    endforeach()
    foreach(mode nonpower owned-nonpower)
      add_test(NAME cq_mpi_${mode}_3
        COMMAND ${MPIEXEC_EXECUTABLE} ${MPIEXEC_NUMPROC_FLAG} 3 ${MPIEXEC_PREFLAGS}
          $<TARGET_FILE:cq_test_lifecycle> ${MPIEXEC_POSTFLAGS} ${mode})
      set_tests_properties(cq_mpi_${mode}_3 PROPERTIES TIMEOUT 60 PROCESSORS 3 ENVIRONMENT "OMP_NUM_THREADS=2")
    endforeach()
  else()
    foreach(mode owned foreign-quest)
      add_test(NAME cq_serial_${mode} COMMAND cq_test_lifecycle ${mode})
      set_tests_properties(cq_serial_${mode} PROPERTIES TIMEOUT 30)
    endforeach()
  endif()
endif()
