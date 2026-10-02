add_executable(test_vqe test_vqe_runner.c test_vqe.c)
target_link_libraries(test_vqe PRIVATE cq-simbe-test-support
  QuEST::QuEST ${CQ_SIMBE_MATH_LIBRARY})
add_test(NAME test_vqe COMMAND test_vqe)
set_tests_properties(test_vqe PROPERTIES TIMEOUT 120)
