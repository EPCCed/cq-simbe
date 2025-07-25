add_executable(vqe
  vqe.c
  vqe_utils.c
)

find_library(MATHS_LIBRARY m REQUIRED)
find_package(NLopt REQUIRED)

target_link_libraries(vqe 
  PRIVATE
  CQ-SIMBE::cq-simbe
  ${MATHS_LIBRARY}
  ${NLopt}
)

