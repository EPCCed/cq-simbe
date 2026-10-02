target_sources(cq-simbe
  PRIVATE
  comms_core.c
)

if(CQ_SIMBE_ENABLE_MPI)
    target_sources(cq-simbe
      PRIVATE
      comms_mpi.c
    )
else()
    target_sources(cq-simbe
      PRIVATE
      comms.c
    )
endif()
