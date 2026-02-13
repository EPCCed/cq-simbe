if(BUILD_FORTRAN_INTERFACE)
  target_sources(cq-fort
    PRIVATE
    host_int.f90
    host_ops.f90
  )
endif()
