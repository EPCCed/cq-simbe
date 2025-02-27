#ifndef CQ_DATATYPES_H
#define CQ_DATATYPES_H

#include <stddef.h>
#include <stdbool.h>

typedef unsigned short int qubit;
typedef short int cstate;
typedef unsigned int backend_id;

typedef struct exec {
  bool complete;
  unsigned int qdev_status;
  size_t completed_shots;
} exec;

typedef void (*qkern)(const size_t NQUBITS, qubit * qreg, cstate * creg);

typedef void (*pqkern)(const size_t NQUBITS, qubit * qreg, cstate * creg, void * params);

#endif