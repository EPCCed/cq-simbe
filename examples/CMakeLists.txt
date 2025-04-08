add_executable(qft
  qft.c
)

find_library(MATHS_LIBRARY m REQUIRED)

target_link_libraries(qft 
  PRIVATE
  CQ-SIMBE::cq-simbe
  ${MATHS_LIBRARY}
)