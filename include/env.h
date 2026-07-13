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

// cq_status cq_init_custom_mpi_comm(int foo, const unsigned int VERBOSITY);

cq_status cq_finalise(const unsigned int VERBOSITY);

#endif
