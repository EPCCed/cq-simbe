#include "datatypes.h"
#include "utils.h"
#include <stdio.h>

void init_creg(const size_t LENGTH, const cstate INIT_VAL, cstate * cr) {
  for (size_t i = 0; i < LENGTH; ++i) {
    cr[i] = INIT_VAL;
  }
  return;
}

void report_results(
  cstate const * const CR,
  const size_t NMEASURE,
  const size_t NSHOTS) {
  printf("Reporting measurement outcomes:\n");
  for (size_t shot = 0; shot < NSHOTS; ++shot) {
    printf("Shot [%lu]: ", shot);
    for (size_t i = shot * NMEASURE; i < (shot + 1) * NMEASURE; ++i) {
      printf("%d ", CR[i]);
    }
    printf("\n");
  }

  return;
}