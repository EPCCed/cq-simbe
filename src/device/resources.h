#ifndef DEVICE_RESOURCES_H
#define DEVICE_RESOURCES_H

#include <stddef.h>
#include <stdbool.h>
#include "datatypes.h"
#include "quest/include/qureg.h"

#define __DEVICE_MAX_NUM_QUREGS__ 64

typedef struct quantumregistry {
  size_t num_registers;
  Qureg registers[__DEVICE_MAX_NUM_QUREGS__];
  bool available[__DEVICE_MAX_NUM_QUREGS__];
} quantumregistry;

extern quantumregistry qregistry;

typedef struct device_alloc_params {
  const size_t NQUBITS;
  size_t qregistry_idx;
  cq_status status;
} device_alloc_params;

void init_qregistry(void);

void clear_qregistry(void);

int get_next_available_qregistry_slot(void);

cq_status device_alloc_qureg(void *);

cq_status device_dealloc_qureg(void *);

#endif