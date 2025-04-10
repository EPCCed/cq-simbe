#ifndef CQ_DATATYPES_H
#define CQ_DATATYPES_H

#include <stddef.h>
#include <stdbool.h>

#define __MAX_NUM_QKERN__ 256
#define __MAX_QKERN_NAME_LENGTH__ 1024

typedef enum cq_status {
  CQ_ERROR = -1,
  CQ_SUCCESS = 0,
  CQ_WARNING = 1
} cq_status;

typedef short int cstate;
typedef unsigned int backend_id;

typedef struct qubit {
  size_t registry_index;
  size_t offset;
  size_t N;
} qubit;

typedef struct exec {
  bool complete;
  unsigned int qdev_status;
  size_t completed_shots;
} exec;

struct qkern_map;
struct pqkern_map;

typedef void (*qkern)
  (const size_t NQUBITS, qubit * qreg, cstate * creg, 
  struct qkern_map * registration);

typedef void (*pqkern)
  (const size_t NQUBITS, qubit * qreg, cstate * creg, void * params, 
  struct pqkern_map * registration);

typedef struct qkern_map {
  qkern fn;
  char fname[__MAX_QKERN_NAME_LENGTH__];
} qkern_map;

typedef struct pqkern_map {
  pqkern fn;
  char fname[__MAX_QKERN_NAME_LENGTH__];
} pqkern_map;

#endif