#ifndef CQ_UTILS_H
#define CQ_UTILS_H

#include "datatypes.h"

void init_creg(const size_t LENGTH, const cstate INIT_VAL, cstate * cr);

void report_results(cstate const * const CR,
                    const size_t NMEASURE,
                    const size_t NSHOTS);

#define HANDLE_CQ_ERROR(x)             \
  {                                    \
    if (x == CQ_ERROR) {               \
      printf(" From: %s\n", __func__); \
      return CQ_ERROR;                 \
    }                                  \
  };

#endif
