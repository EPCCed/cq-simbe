foreach(group IN ITEMS host_ops device_ops qasm_gates)
  set(target "test_fort_${group}")
  add_executable(${target} "test_${group}.f90" utils.f90)
  target_link_libraries(${target} PRIVATE CQ-SIMBE::fortran)
  set_target_properties(${target} PROPERTIES
    Fortran_PREPROCESS ON
    Fortran_MODULE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/mod/${target}"
  )
  if(CQ_SIMBE_ENABLE_MPI)
    add_test(NAME ${target}
      COMMAND "${MPIEXEC_EXECUTABLE}" "${MPIEXEC_NUMPROC_FLAG}" 2
        ${MPIEXEC_PREFLAGS} $<TARGET_FILE:${target}> ${MPIEXEC_POSTFLAGS})
    set_tests_properties(${target} PROPERTIES PROCESSORS 2)
  else()
    add_test(NAME ${target} COMMAND ${target})
  endif()
  set_tests_properties(${target} PROPERTIES TIMEOUT 120)
endforeach()
