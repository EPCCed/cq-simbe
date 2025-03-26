#ifndef CQ_ENV_H
#define CQ_ENV_H

#include <stdbool.h>
#include <pthread.h>
#include "datatypes.h"

struct cq_environment {
  bool initialised;
  bool finalised;
};

extern struct cq_environment cq_env;

cq_status cq_init(const unsigned int VERBOSITY);

cq_status cq_finalise(const unsigned int VERBOSITY);

#endif