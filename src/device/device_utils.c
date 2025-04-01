#include "device_utils.h"
#include "datatypes.h"
#include "quest/include/types.h"

#ifndef NDEBUG
#include <stdio.h>
#endif

cq_status qindex_to_cstate(qindex state, cstate * creg, const size_t BIT_WIDTH)
{
  cq_status status = CQ_ERROR;

  for (int i = BIT_WIDTH-1; i >= 0; --i) {
    creg[i] = state % 2;
    state /= 2;
  }

  if (state == 0) status = CQ_SUCCESS;

  return status;
}