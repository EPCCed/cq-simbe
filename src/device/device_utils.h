#ifndef CQ_DEVICE_UTILS_H
#define CQ_DEVICE_UTILS_H

#include "datatypes.h"
#include "quest/include/types.h"

int qindex_to_cstate(qindex state, cstate * creg, const size_t BIT_WIDTH);

#endif