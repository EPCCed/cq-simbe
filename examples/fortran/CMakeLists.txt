find_library(MATHS_LIBRARY m REQUIRED)

add_executable(zero-qft
  zero_qft.f90
  utils.f90
)

target_link_libraries(zero-qft
  PRIVATE
  CQ-SIMBE::cq-simbe
  ${MATHS_LIBRARY}
)

set_target_properties(zero-qft PROPERTIES Fortran_PREPROCESS ON)

add_executable(plus-qft
  plus_qft.f90
  utils.f90
)

target_link_libraries(plus-qft
  PRIVATE
  CQ-SIMBE::cq-simbe
  ${MATHS_LIBRARY}
)

set_target_properties(plus-qft PROPERTIES Fortran_PREPROCESS ON)
