#ifndef CQ_DATATYPES_H
#define CQ_DATATYPES_H

#include <stddef.h>
#include <stdbool.h>
#include <pthread.h>

#define __CQ_MAX_NUM_QKERN__ 256
#define __CQ_MAX_QKERN_NAME_LENGTH__ 1024

struct qkern_params;

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
  bool exec_init;
  bool complete;
  cq_status status;
  size_t completed_shots;
  size_t expected_shots;
  pthread_mutex_t lock;
  pthread_cond_t cond_exec_complete;
  struct qkern_params * qk_pars;
} cq_exec;

struct qkern_map;
struct pqkern_map;

typedef cq_status (*qkern)
  (const size_t NQUBITS, qubit * qreg, cstate * creg, 
  struct qkern_map * registration);

typedef cq_status (*pqkern)
  (const size_t NQUBITS, qubit * qreg, cstate * creg, void * params, 
  struct pqkern_map * registration);

typedef struct qkern_map {
  qkern fn;
  char fname[__CQ_MAX_QKERN_NAME_LENGTH__];
} qkern_map;

typedef struct pqkern_map {
  pqkern fn;
  char fname[__CQ_MAX_QKERN_NAME_LENGTH__];
} pqkern_map;

#endif