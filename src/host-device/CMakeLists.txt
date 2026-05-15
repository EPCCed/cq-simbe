add_subdirectory(comms)

target_sources(cq-simbe
  PRIVATE
#  comms.c
  kernel_utils.c
#  mpi_comms.c
)
