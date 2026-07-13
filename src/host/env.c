#include "env.h"
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include "datatypes.h"
#include "opcodes.h"
#include "src/host-device/comms.h"

struct cq_environment cq_env = { .initialised = false, .finalised = false };

struct dev_link dev_ctrl;

cq_status cq_init(const unsigned int VERBOSITY) {
  cq_status status = CQ_SUCCESS;

  if (!cq_env.finalised) {
    if (!cq_env.initialised) {
      if (VERBOSITY > 0) {
        printf(
            "Initialising CQ Simulated Backend library. QuEST environment "
            "report to follow.\n\n");
      }

      initialise_device(VERBOSITY);

      cq_env.initialised = true;
    } else {
      if (VERBOSITY > 0) {
        printf("CQ-SimBE is already initialised. No need to do it again.\n");
      }
      status = CQ_WARNING;
    }
  } else {
    printf(
        "CQ-SimBE cannot be reinitialised once finalised! This would break "
        "QuEST.\n");
    status = CQ_ERROR;
  }

  return status;
}

#if CQ_WITH_MPI_COMMS
cq_status cq_init_custom_mpi_comm(MPI_Comm cq_comm,
                                  const unsigned int VERBOSITY) {
  cq_status status = CQ_SUCCESS;

  if (!cq_env.finalised) {
    if (!cq_env.initialised) {
      if (VERBOSITY > 0) {
        printf(
            "Initialising CQ Simulated Backend library. QuEST environment "
            "report to follow.\n\n");
      }

      initialise_device_with_custom_mpi_comm(cq_comm, VERBOSITY);

      cq_env.initialised = true;
    } else {
      if (VERBOSITY > 0) {
        printf("CQ-SimBE is already initialised. No need to do it again.\n");
      }
      status = CQ_WARNING;
    }
  } else {
    printf(
        "CQ-SimBE cannot be reinitialised once finalised! This would break "
        "QuEST.\n");
    status = CQ_ERROR;
  }

  return status;
}

#endif

cq_status cq_finalise(const unsigned int VERBOSITY) {
  cq_status status = CQ_SUCCESS;

  if (!cq_env.finalised) {
    if (VERBOSITY > 0) printf("Host finalising\n");

    finalise_device(VERBOSITY);
    cq_env.finalised = true;
  } else {
    if (VERBOSITY > 0) {
      printf("CQ-SimBE is already finalised. No need to do it again.\n");
    }
    status = CQ_WARNING;
  }

  return status;
}
