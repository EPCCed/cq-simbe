#include "datatypes.h"
#include "utils.h"

#include <stdio.h>

void init_creg(const size_t LENGTH, const cstate INIT_VAL, cstate * cr) {
  printf("%s: LENGTH=%lu, INIT_VAL=%d\n", __func__, LENGTH, INIT_VAL);
  for (size_t i = 0; i < LENGTH; ++i) {
    cr[i] = INIT_VAL;
    printf("\tcr[%lu] = %d\n", i, cr[i]);
  }
  
  return;
}