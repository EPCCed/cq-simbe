if(BUILD_FORTRAN_INTERFACE)
  target_sources(cq-fort
    PRIVATE
    device_int.f90
    device_ops.f90
    qasm_gates.f90
  )
endif()
