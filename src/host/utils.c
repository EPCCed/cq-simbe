#include "datatypes.h"
#include "utils.h"

void init_creg(const size_t LENGTH, const cstate INIT_VAL, cstate * cr) {
  for (size_t i = 0; i < LENGTH; ++i) {
    cr[i] = INIT_VAL;
  }
  return;
}