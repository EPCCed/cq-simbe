target_sources(cq-simbe
  PRIVATE
  comms_core.c
)

if (BUILD_WITH_MPI)
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
