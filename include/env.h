#ifndef CQ_ENV_H
#define CQ_ENV_H

#include <stdbool.h>
#include <pthread.h>

int cq_init(const unsigned int VERBOSITY);

int cq_finalise();

#endif