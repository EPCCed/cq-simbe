target_sources(cq-simbe
  PRIVATE
  env.c
  host_ops.c
  utils.c
)

if(BUILD_FORTRAN_INTERFACE)
add_subdirectory(fortran)
endif()
