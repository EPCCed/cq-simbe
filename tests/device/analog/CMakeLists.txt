foreach(suite IN ITEMS analog waveforms)
  add_executable(test_${suite} test_${suite}.c test_${suite}_runner.c)
  target_link_libraries(test_${suite} PRIVATE cq-simbe-test-support
    QuEST::QuEST ${CQ_SIMBE_MATH_LIBRARY})
  add_test(NAME test_${suite} COMMAND test_${suite})
  set_tests_properties(test_${suite} PROPERTIES TIMEOUT 120)
endforeach()
