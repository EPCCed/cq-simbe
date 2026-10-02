add_executable(qaa qaa.c)
target_link_libraries(qaa PRIVATE CQ-SIMBE::cq-simbe ${CQ_SIMBE_MATH_LIBRARY})
