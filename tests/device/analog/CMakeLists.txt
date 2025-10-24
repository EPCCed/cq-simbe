find_library(MATHS_LIBRARY m REQUIRED)

# ------------------------ analog ops -----------------------------
add_executable(test_analog
  test_analog.c
  test_analog_runner.c
)

target_link_libraries(test_analog
  PRIVATE
  CQ-SIMBE::cq-simbe
  unity::framework
  QuEST::QuEST
)

add_test(
  NAME test_analog
  COMMAND ./test_analog
)

# ------------------------ waveforms ------------------------------
add_executable(test_waveforms
  test_waveforms.c
  test_waveforms_runner.c
)

target_link_libraries(test_waveforms
  PRIVATE
  CQ-SIMBE::cq-simbe
  unity::framework
)

add_test(
  NAME test_waveforms
  COMMAND ./test_waveforms
)

