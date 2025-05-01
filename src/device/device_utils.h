#ifndef CQ_DEVICE_UTILS_H
#define CQ_DEVICE_UTILS_H

#include "datatypes.h"
#include "quest/include/types.h"

cq_status qindex_to_cstate(qindex state, cstate * creg, const size_t BIT_WIDTH);

cq_status cstate_to_qindex(cstate const * const CREG, qindex * state, const size_t BIT_WIDTH);

#endif