find_library(MATHS_LIBRARY m REQUIRED)

add_library(fort-example-utils utils.f90)
target_link_libraries(fort-example-utils cq-fort)

add_executable(zero-qft
  zero_qft.f90
)

target_link_libraries(zero-qft
  PRIVATE
  CQ-SIMBE::cq-simbe
  cq-fort
  fort-example-utils
  ${MATHS_LIBRARY}
)

add_executable(plus-qft
  plus_qft.f90
)

target_link_libraries(plus-qft
  PRIVATE
  CQ-SIMBE::cq-simbe
  cq-fort
  fort-example-utils
  ${MATHS_LIBRARY}
)

add_executable(heat-eq
  heat.f90
)

target_link_libraries(heat-eq
  PRIVATE
  CQ-SIMBE::cq-simbe
  cq-fort
  fort-example-utils
  ${MATHS_LIBRARY}
)
