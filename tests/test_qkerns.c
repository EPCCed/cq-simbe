#include <math.h>
#include <string.h>
#include <stdio.h>
#include "datatypes.h"
#include "kernel_utils.h"
#include "device_ops.h"
#include "qasm_gates.h"
#include "test_qkerns.h"

#ifndef M_PI
#define M_PI 3.1415926535897932384
#endif

void qft_circuit(const size_t NQUBITS, qubit * qr) {
  // Run QFT
  for (size_t i = 0; i < NQUBITS; ++i) {
    hadamard(&qr[i]); 
    for (size_t j = i+1; j < NQUBITS; ++j) {
      double angle = M_PI / pow(2, j);
      cphase(&qr[j], &qr[i], angle);
    }
  }


  for (size_t i = 0; i < NQUBITS / 2; ++i) {
    size_t j = NQUBITS - (i+1);
    swap(&qr[i], &qr[j]);
  } 

  return;
}


void zero_init_full_qft(
const size_t NQUBITS, qubit * qr, cstate * cr, qkern_map * reg) {
  CQ_REGISTER_KERNEL(reg)

  // Prepare state
  set_qureg(qr, 0, NQUBITS);

  // Run QFT
  qft_circuit(NQUBITS, qr);

  // Measure
  measure_qureg(qr, NQUBITS, cr);
  
  return;
}

void equal_superposition_full_qft(
const size_t NQUBITS, qubit * qr, cstate * cr, qkern_map * reg) {
  CQ_REGISTER_KERNEL(reg);

  // Prepare state
  set_qureg(qr, 0, NQUBITS);
  for (size_t i = 0; i < NQUBITS; ++i) {
    hadamard(&qr[i]);
  }

  // Run QFT
  qft_circuit(NQUBITS, qr);

  // Measure
  measure_qureg(qr, NQUBITS, cr);

  return;
}

void all_site_hadamard(const size_t NQUBITS, qubit * qr, cstate * cr,
qkern_map * reg) {
  CQ_REGISTER_KERNEL(reg);

  set_qureg(qr, 0, NQUBITS);
  for (size_t i = 0; i < NQUBITS; ++i) {
    hadamard(&qr[i]);
  }

  measure_qureg(qr, NQUBITS, cr);
  
  return;
}

void only_measure_first_site(const size_t NQUBITS, qubit * qr,
cstate * cr, qkern_map * reg) {
  CQ_REGISTER_KERNEL(reg);

  set_qureg(qr, 0, NQUBITS);

  hadamard(&qr[0]);

  // TODO: implement with measure_qubit once implemented
  measure_qureg(qr, 1, cr);

  return;
}

void no_measure_qkern(const size_t NQUBITS, qubit * qr, cstate * cr,
qkern_map * reg) {
  CQ_REGISTER_KERNEL(reg);

  // a tree falls in the forest with no-one to hear it...

  return;
}

void unregistered_kernel(const size_t NQUBITS, qubit * qr,
cstate * cr, qkern_map * reg) {
  printf("I'm an unregistered kernel. How did you get here?\n");
  return;
}

void overly_long_qkern_name(const size_t NQUBITS, qubit * qr,
cstate * cr, qkern_map * reg) {
  // "overly_long_qkern_name" is of course fine, but the string
  // we're about to build isn't!
  char too_long[__CQ_MAX_QKERN_NAME_LENGTH__ + 1];
  memset(too_long, 'a', __CQ_MAX_QKERN_NAME_LENGTH__);
  too_long[__CQ_MAX_QKERN_NAME_LENGTH__] = '\0';

  // can't just use the macro here as it explicitly calls __func__ and I
  // didn't want to have to write a longer than __CQ_MAX_QKERN_NAME_LENGTH__ function name
  // this is stupid as we have to replicate CQ_REGISTER_KERNEL here and update if it changes
  // and if we don't remember to do that we're not really testing an overly long qkern name.
  // So sue me. (TODO: Check license terms mean I can't be sued.)
  if (reg != NULL) {
    size_t strsz = sizeof(too_long);
    if (strsz < __CQ_MAX_QKERN_NAME_LENGTH__) {
      strcpy(reg->fname, too_long);
    } else {
      reg->fname[0] = '\0';
    }
    return;
  }

  return;
}