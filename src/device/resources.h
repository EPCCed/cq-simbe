#ifndef DEVICE_RESOURCES_H
#define DEVICE_RESOURCES_H

#include <stddef.h>
#include <stdbool.h>
#include "quest/include/qureg.h"

#define __DEVICE_MAX_NUM_QUREGS__ 64

typedef struct quantumregistry {
  size_t num_registers;
  Qureg registers[__DEVICE_MAX_NUM_QUREGS__];
  bool available[__DEVICE_MAX_NUM_QUREGS__];
} quantumregistry;

void device_alloc_qureg(const size_t NQUBITS);

#endif