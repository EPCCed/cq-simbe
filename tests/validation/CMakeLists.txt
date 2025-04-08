find_library(MATHS_LIBRARY m REQUIRED)

add_executable(test_qft
  test_qft_runner.c
  test_qft.c
)

target_link_libraries(test_qft
  PRIVATE
  CQ-SIMBE::cq-simbe
  unity::framework
  QuEST::QuEST
  ${MATHS_LIBRARY}
)

add_test(
  NAME test_qft
  COMMAND ./test_qft
)