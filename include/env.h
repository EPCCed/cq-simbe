#ifndef CQ_ENV_H
#define CQ_ENV_H

#include "datatypes.h"

#include <pthread.h>
#include <stdbool.h>

struct cq_environment {
  bool initialised;
  bool finalised;
};

extern struct cq_environment cq_env;

cq_status cq_init(const unsigned int VERBOSITY);

#if CQ_WITH_MPI_COMMS
#include <mpi.h>
cq_status cq_init_custom_mpi_comm(MPI_Comm cq_comm,
                                  const unsigned int VERBOSITY);
#endif

cq_status cq_finalise(const unsigned int VERBOSITY);

#endif
