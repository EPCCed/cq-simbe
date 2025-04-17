#include <complex.h>
#include <math.h>
#include "unity.h"
#include "datatypes.h"
#include "host_ops.h"
#include "device_ops.h"
#include "src/device/resources.h"
#include "qasm_gates.h"
#include "test_qasm_gates.h"

#ifndef M_PI
#define M_PI 3.141592653589793238
#endif

const double DELTA = __DBL_EPSILON__;
const size_t NQUBITS = 5;
const size_t NAMPS = 1 << NQUBITS;
qubit * qr;

void setUp(void) {
  alloc_qureg(&qr, NQUBITS);  
  return;
}

void tearDown(void) {
  free_qureg(&qr);
  return;
}

void init_plus_state(void) {
  set_qureg(qr, 0, NQUBITS);
  for (size_t i = 0; i < NQUBITS; ++i) {
    hadamard(&qr[i]);
  }
  return;
}

int check_bit(unsigned long long state, unsigned long long position) {
  return state & (1LLU << position);
}

unsigned int pop_count(unsigned long long value) {
  // courtesy of Brian Kernighan
  unsigned int count;

  // Bitwise AND of value and value-1 clears the least significant set bit
  for (count = 0; value != 0; ++count) {
    value &= value - 1;
  }
  
  return count;
}

void getAmps(complex double * sv) {
  getQuregAmps(sv, qregistry.registers[qr[0].registry_index], 0, NAMPS);
  return;
}

void test_unitary(void) {
  const double EXPECTED = 1.0 / sqrt(NAMPS);
  complex double sv[NAMPS];
  char msg[64];
  double theta, phi, lambda;
  size_t i;

  init_plus_state();

  // Identity
  theta = 0.0;
  phi = 0.0;
  lambda = 0.0;

  for (i = 0; i < NQUBITS; ++i) {
    TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, unitary(&qr[i], theta, phi, lambda));
  }

  getAmps(sv);
  for (i = 0; i < NAMPS; ++i) {
    sprintf(msg, "Real component: index = %lu", i);
    TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(EXPECTED, creal(sv[i]), msg);

    sprintf(msg, "Imaginary component: index = %lu", i);
    TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(0.0, cimag(sv[i]), msg);
  } 
  
  set_qureg(qr, 0, NQUBITS);

  // NOT
  theta = M_PI;
  phi = 3 * M_PI / 2;
  lambda = M_PI / 2;

  for (i = 0; i < NQUBITS; ++i) {
    TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, unitary(&qr[i], theta, phi, lambda));
  }

  getAmps(sv);
  TEST_ASSERT_EACH_EQUAL_DOUBLE(0.0, sv, 2 * (NAMPS - 1));
  TEST_ASSERT_EQUAL_DOUBLE(1.0, creal(sv[NAMPS-1]));
  TEST_ASSERT_EQUAL_DOUBLE(0.0, cimag(sv[NAMPS-1]));

  return;
}

void test_gphase(void) {
  // it shouldn't matter what qubit this is applied to
  size_t i;
  char msg[64];
  complex double sv[NAMPS];
  const double EXPECTED = 1.0 / sqrt(NAMPS);

  init_plus_state();
  
  // * -1
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, gphase(&qr[0], M_PI));
  getAmps(sv);
  for (i = 0; i < NAMPS; ++i) {
    sprintf(msg, "Real component: index = %lu", i);
    TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(-EXPECTED, creal(sv[i]), msg);

    sprintf(msg, "Imaginary component: index = %lu", i);
    TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(0.0, cimag(sv[i]), msg);
  } 
  
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, gphase(&qr[NQUBITS-1], M_PI));
  getAmps(sv);
  for (i = 0; i < NAMPS; ++i) {
    sprintf(msg, "Real component: index = %lu", i);
    TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(EXPECTED, creal(sv[i]), msg);

    sprintf(msg, "Imaginary component: index = %lu", i);
    TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(0.0, cimag(sv[i]), msg);
  }
  
  // * i
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, gphase(&qr[0], M_PI/2));
  getAmps(sv);
  for (i = 0; i < NAMPS; ++i) {
    sprintf(msg, "Real component: index = %lu", i);
    TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(0.0, creal(sv[i]), msg);

    sprintf(msg, "Imaginary component: index = %lu", i);
    TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(EXPECTED, cimag(sv[i]), msg);
  }

  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, gphase(&qr[NQUBITS-1], M_PI/2));
  getAmps(sv);
  for (i = 0; i < NAMPS; ++i) {
    sprintf(msg, "Real component: index = %lu", i);
    TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(-EXPECTED, creal(sv[i]), msg);

    sprintf(msg, "Imaginary component: index = %lu", i);
    TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(0.0, cimag(sv[i]), msg);
  }

  return;
}

void test_phase(void) {
  char msg[64];
  complex double sv[NAMPS];
  const double EXPECTED = 1.0 / sqrt(NAMPS);

  init_plus_state();

  // * i
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, phase(&qr[0], M_PI/2));
  getAmps(sv);
  for (size_t i = 0; i < NAMPS; ++i) {
    sprintf(msg, "Real component: index = %lu", i);
    if (check_bit(i, 0)) {
      TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(0.0, creal(sv[i]), msg);
    } else {
      TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(EXPECTED, creal(sv[i]), msg);
    }

    sprintf(msg, "Imaginary component: index = %lu", i);
    if (check_bit(i, 0)) {
      TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(EXPECTED, cimag(sv[i]), msg);
    } else {
      TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(0.0, cimag(sv[i]), msg);
    }
  }

  return;
}

void test_hadamard(void) {
  const double EXPECTED = 1.0 / sqrt(NAMPS);
  char msg[64];
  complex double sv[NAMPS];
  
  set_qureg(qr, 0, NQUBITS);
  for (size_t i = 0; i < NQUBITS; ++i) {
    TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, hadamard(&qr[i]));
  }

  getAmps(sv);
  for (size_t i = 0; i < NAMPS; ++i) {
    sprintf(msg, "Real component: index = %lu", i);
    TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(EXPECTED, creal(sv[i]), msg);

    sprintf(msg, "Imaginary component: index = %lu", i);
    TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(0.0, cimag(sv[i]), msg);
  }

  for (size_t i = 0; i < NQUBITS; ++i) {
    TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, hadamard(&qr[i]));
  }

  getAmps(sv);
  TEST_ASSERT_EQUAL_DOUBLE(1.0, creal(sv[0]));
  TEST_ASSERT_EQUAL_DOUBLE(0.0, cimag(sv[0]));
  for (size_t i = 1; i < NAMPS; ++i) {
    sprintf(msg, "Real component: index = %lu", i);
    TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(0.0, creal(sv[i]), msg);

    sprintf(msg, "Imaginary component: index = %lu", i);
    TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(0.0, cimag(sv[i]), msg);
  }

  TEST_ASSERT_EQUAL_INT(CQ_ERROR, hadamard(NULL));
  
  return;
}

void test_cphase(void) {
  char msg[128];
  complex double sv[NAMPS];
  const double EXPECTED = 1.0 / sqrt(NAMPS);
  
  // exp(i*pi/2) = i
  const double ANGLE = M_PI/2;

  // initialise to |+++++>
  init_plus_state();
  
  getAmps(sv);
  for (size_t i = 0; i < NAMPS; ++i) {
    sprintf(msg, "Real component: index = %lu", i);
    TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(EXPECTED, creal(sv[i]), msg);

    sprintf(msg, "Imaginary component: index = %lu", i);
    TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(0.0, cimag(sv[i]), msg);
  }

  // We expect the amplitude to have moved to the imaginary component for the amplitudes
  // where both the control and target qubit are 1, otherwise no change.
  for (size_t ctrl = 0; ctrl < NQUBITS; ++ctrl) {
    for (size_t target = 0; target < NQUBITS; ++target) {
      if (ctrl != target) {
        TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, cphase(&qr[ctrl], &qr[target], ANGLE));
        getAmps(sv);
        for (size_t state = 0; state < NAMPS; ++state) {
          if (check_bit(state, ctrl) && check_bit(state, target)) {
            sprintf(msg, "Real component: index = %lu, control = %lu, target = %lu", state, ctrl, target);
            TEST_ASSERT_DOUBLE_WITHIN_MESSAGE(DELTA, 0.0, creal(sv[state]), msg);
            sprintf(msg, "Imaginary component: index = %lu, control = %lu, target = %lu", state, ctrl, target);
            TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(EXPECTED, cimag(sv[state]), msg);
          } else {
            sprintf(msg, "Real component: index = %lu, control = %lu, target = %lu", state, ctrl, target);
            TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(EXPECTED, creal(sv[state]), msg);
            sprintf(msg, "Imaginary component: index = %lu, control = %lu, target = %lu", state, ctrl, target);
            TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(0.0, cimag(sv[state]), msg);
          }
        }
        // Reset to plus state
        init_plus_state();
      } else {
        TEST_ASSERT_EQUAL_INT(CQ_ERROR, cphase(&qr[ctrl], &qr[target], ANGLE));
      }
    }
  }

  TEST_ASSERT_EQUAL_INT(CQ_ERROR, cphase(&qr[0], NULL, 0.0));
  TEST_ASSERT_EQUAL_INT(CQ_ERROR, cphase(NULL, &qr[0], 0.0));
  TEST_ASSERT_EQUAL_INT(CQ_ERROR, cphase(NULL, NULL, 0.0)); 

  return;
}

void test_swap(void) {
  char msg[128];
  complex double sv[NAMPS];
  const double EXPECTED = 1.0 / sqrt(2);

  set_qureg(qr, 0, NQUBITS);
  hadamard(&qr[0]);

  for (size_t hot_qubit = 0; hot_qubit <= NQUBITS; ++hot_qubit) {
    getAmps(sv);
    // We expect amplitude in the all-zero state, and in the state where 
    // only the target qubit is 1

    size_t hqi = hot_qubit % NQUBITS;
    for (size_t i = 0; i < NAMPS; ++i) {
      if (i == 0 || (check_bit(i, hqi) && pop_count(i) == 1)) {
        sprintf(msg, "Real component: index = %lu", i);
        TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(EXPECTED, creal(sv[i]), msg);
        sprintf(msg, "Imaginary component: index = %lu", i);
        TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(0.0, cimag(sv[i]), msg);
      } else {
        sprintf(msg, "Real component: index = %lu", i);
        TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(0.0, creal(sv[i]), msg);
        sprintf(msg, "Imaginary component: index = %lu", i);
        TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(0.0, cimag(sv[i]), msg);
      }
    }

    TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, swap(&qr[hqi], &qr[(hot_qubit+1) % NQUBITS]));
  }

  TEST_ASSERT_EQUAL_INT(CQ_ERROR, swap(&qr[0], &qr[0]));
  TEST_ASSERT_EQUAL_INT(CQ_ERROR, swap(&qr[0], NULL));
  TEST_ASSERT_EQUAL_INT(CQ_ERROR, swap(NULL, &qr[0]));
  TEST_ASSERT_EQUAL_INT(CQ_ERROR, swap(NULL, NULL));

  return;
}