add_executable(qaa
  qaa.c
)

find_library(MATHS_LIBRARY m REQUIRED)

target_link_libraries(qaa
  PRIVATE
  CQ-SIMBE::cq-simbe
  ${MATHS_LIBRARY}
)
