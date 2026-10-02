add_executable(vqe vqe.c vqe_utils.c)
target_link_libraries(vqe PRIVATE CQ-SIMBE::cq-simbe ${CQ_SIMBE_MATH_LIBRARY} NLopt::nlopt)
