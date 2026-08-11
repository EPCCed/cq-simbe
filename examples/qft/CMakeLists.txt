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

if (BUILD_WITH_MPI)
  add_executable(mpi_aqft
    mpi_aqft.c
    qft-utils.c
  )
  
  find_library(MATHS_LIBRARY m REQUIRED)
  
  target_link_libraries(mpi_aqft 
    PRIVATE
    CQ-SIMBE::cq-simbe
    ${MATHS_LIBRARY}
    ${MPI_C_LIBRARIES}
  )
endif()
