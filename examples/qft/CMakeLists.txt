add_executable(sqft
  sqft.c
  qft-utils.c
)


target_link_libraries(sqft 
  PRIVATE
  CQ-SIMBE::cq-simbe
  ${CQ_SIMBE_MATH_LIBRARY}
)

add_executable(aqft
  aqft.c
  qft-utils.c
)


target_link_libraries(aqft 
  PRIVATE
  CQ-SIMBE::cq-simbe
  ${CQ_SIMBE_MATH_LIBRARY}
)
