find_library(MATHS_LIBRARY m REQUIRED)

add_executable(test_vqe
  test_vqe_runner.c
  test_vqe.c
)

target_link_libraries(test_vqe
  PRIVATE
  CQ-SIMBE::cq-simbe
  unity::framework
  QuEST::QuEST
  ${MATHS_LIBRARY}
)

add_test(
  NAME test_vqe
  COMMAND ./test_vqe
)


