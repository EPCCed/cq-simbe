add_executable(sqft
  sqft.c
  qft-utils.c
)

find_library(MATHS_LIBRARY m REQUIRED)

target_link_libraries(sqft 
  PRIVATE
  CQ-SIMBE::cq-simbe
  ${MATHS_LIBRARY}
)

add_executable(aqft
  aqft.c
  qft-utils.c
)

find_library(MATHS_LIBRARY m REQUIRED)

target_link_libraries(aqft 
  PRIVATE
  CQ-SIMBE::cq-simbe
  ${MATHS_LIBRARY}
)