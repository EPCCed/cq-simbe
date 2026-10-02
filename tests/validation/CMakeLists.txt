add_executable(test_qft
  test_qft_runner.c
  test_qft.c
  ../test_qkerns.c
)

target_link_libraries(test_qft
  PRIVATE
  cq-simbe-test-support
  QuEST::QuEST
  ${CQ_SIMBE_MATH_LIBRARY}
)

add_test(
  NAME test_qft
  COMMAND test_qft
)
