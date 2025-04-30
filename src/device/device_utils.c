#include "device_utils.h"
#include "datatypes.h"
#include "quest/include/types.h"

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

cq_status cstate_to_qindex(cstate const * const CREG, qindex * state, const size_t BIT_WIDTH) {
  *state = 0;

  for (size_t i = 0; i < BIT_WIDTH; ++i) {
    if (CREG[i] > -1 && CREG[i] < 2) {
      *state += (1lu << i) * CREG[i];
    } else {
      *state = -1;
      return CQ_ERROR;
    }
  }

  return CQ_SUCCESS;
}