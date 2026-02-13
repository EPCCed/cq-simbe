find_library(MATHS_LIBRARY m REQUIRED)

add_library(qft-lib ../qft/qft-utils.c ../qft/sqft.c)

target_link_libraries(qft-lib
  PRIVATE
  CQ-SIMBE::cq-simbe
  ${MATHS_LIBRARY}
)

add_executable(fort-qft
  qft.f90
)

target_link_libraries(fort-qft
  PRIVATE
  CQ-SIMBE::cq-simbe
  cq-fort
  qft-lib
  ${MATHS_LIBRARY}
)

add_executable(test-fort
  tests.f90
)

target_link_libraries(test-fort
  PRIVATE
  CQ-SIMBE::cq-simbe
  cq-fort
  qft-lib
  ${MATHS_LIBRARY}
)


