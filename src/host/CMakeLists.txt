target_sources(cq-simbe
  PRIVATE
  env.c
  host_ops.c
  utils.c
)

if(BUILD_FORTRAN_INTERFACE)
  target_sources(cq-fort
    PRIVATE
    host_int.f90
  )
endif()
