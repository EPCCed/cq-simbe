function(cq_add_fortran_example target source)
  add_executable(${target} ${source} utils.f90)
  target_link_libraries(${target} PRIVATE CQ-SIMBE::fortran ${CQ_SIMBE_MATH_LIBRARY})
  set_target_properties(${target} PROPERTIES
    Fortran_PREPROCESS ON
    Fortran_MODULE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/mod/${target}"
  )
endfunction()

cq_add_fortran_example(zero-qft zero_qft.f90)
cq_add_fortran_example(plus-qft plus_qft.f90)
if(CQ_SIMBE_ENABLE_MPI)
  find_package(MPI REQUIRED COMPONENTS Fortran)
  cq_add_fortran_example(mpi-plus-qft mpi_plus_qft.f90)
  target_link_libraries(mpi-plus-qft PRIVATE MPI::MPI_Fortran)
endif()
