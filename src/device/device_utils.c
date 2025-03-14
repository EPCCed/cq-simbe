#include "device_utils.h"
#include "datatypes.h"
#include "quest/include/types.h"

#include <stdio.h>

int qindex_to_cstate(qindex state, cstate * creg, const size_t BIT_WIDTH) {
  size_t i = BIT_WIDTH-1;
  while (i > 0) {
    creg[i] = state % 2;
    state /= 2;
    --i;
  }

  return state;
}