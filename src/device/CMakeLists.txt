target_sources(cq-simbe
  PRIVATE
  control.c
  device_ops.c
  qasm_gates.c
  resources.c
  device_utils.c
)

add_subdirectory(analog)

if(BUILD_FORTRAN_INTERFACE)
  target_sources(cq-fort
    PRIVATE
    device_int.f90
  )
endif()
