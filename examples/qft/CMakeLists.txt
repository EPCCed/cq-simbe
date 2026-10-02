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

if(CQ_SIMBE_ENABLE_MPI)
  add_executable(mpi_aqft mpi_aqft.c qft-utils.c)
  target_link_libraries(mpi_aqft PRIVATE CQ-SIMBE::cq-simbe ${CQ_SIMBE_MATH_LIBRARY})
endif()
