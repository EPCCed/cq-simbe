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

const double DELTA = 10 * __DBL_EPSILON__;
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

  double expect_zero[2 * (NAMPS-1)];
  for (i = 0; i < 2 * (NAMPS-1); ++i) expect_zero[i] = 0.0;

  getAmps(sv);
  TEST_ASSERT_DOUBLE_ARRAY_WITHIN(DELTA, expect_zero, sv, 2 * (NAMPS-1));
  TEST_ASSERT_DOUBLE_WITHIN(DELTA, 1.0, creal(sv[NAMPS-1]));
  TEST_ASSERT_DOUBLE_WITHIN(DELTA, 0.0, cimag(sv[NAMPS-1]));

  TEST_ASSERT_EQUAL_INT(CQ_ERROR, unitary(NULL, theta, phi, lambda));

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
    TEST_ASSERT_DOUBLE_WITHIN_MESSAGE(DELTA, -EXPECTED, creal(sv[i]), msg);

    sprintf(msg, "Imaginary component: index = %lu", i);
    TEST_ASSERT_DOUBLE_WITHIN_MESSAGE(DELTA, 0.0, cimag(sv[i]), msg);
  } 
  
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, gphase(&qr[NQUBITS-1], M_PI));
  getAmps(sv);
  for (i = 0; i < NAMPS; ++i) {
    sprintf(msg, "Real component: index = %lu", i);
    TEST_ASSERT_DOUBLE_WITHIN_MESSAGE(DELTA, EXPECTED, creal(sv[i]), msg);

    sprintf(msg, "Imaginary component: index = %lu", i);
    TEST_ASSERT_DOUBLE_WITHIN_MESSAGE(DELTA, 0.0, cimag(sv[i]), msg);
  }
  
  // * i
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, gphase(&qr[0], M_PI/2));
  getAmps(sv);
  for (i = 0; i < NAMPS; ++i) {
    sprintf(msg, "Real component: index = %lu", i);
    TEST_ASSERT_DOUBLE_WITHIN_MESSAGE(DELTA, 0.0, creal(sv[i]), msg);

    sprintf(msg, "Imaginary component: index = %lu", i);
    TEST_ASSERT_DOUBLE_WITHIN_MESSAGE(DELTA, EXPECTED, cimag(sv[i]), msg);
  }

  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, gphase(&qr[NQUBITS-1], M_PI/2));
  getAmps(sv);
  for (i = 0; i < NAMPS; ++i) {
    sprintf(msg, "Real component: index = %lu", i);
    TEST_ASSERT_DOUBLE_WITHIN_MESSAGE(DELTA, -EXPECTED, creal(sv[i]), msg);

    sprintf(msg, "Imaginary component: index = %lu", i);
    TEST_ASSERT_DOUBLE_WITHIN_MESSAGE(DELTA, 0.0, cimag(sv[i]), msg);
  }

  TEST_ASSERT_EQUAL_INT(CQ_ERROR, gphase(NULL, 0.0));

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
      TEST_ASSERT_DOUBLE_WITHIN_MESSAGE(DELTA, 0.0, creal(sv[i]), msg);
    } else {
      TEST_ASSERT_DOUBLE_WITHIN_MESSAGE(DELTA, EXPECTED, creal(sv[i]), msg);
    }

    sprintf(msg, "Imaginary component: index = %lu", i);
    if (check_bit(i, 0)) {
      TEST_ASSERT_DOUBLE_WITHIN_MESSAGE(DELTA, EXPECTED, cimag(sv[i]), msg);
    } else {
      TEST_ASSERT_DOUBLE_WITHIN_MESSAGE(DELTA, 0.0, cimag(sv[i]), msg);
    }
  }

  TEST_ASSERT_EQUAL_INT(CQ_ERROR, phase(NULL, 0.0));

  return;
}

void test_paulix(void) {
  complex double sv[NAMPS];

  // init to all zero
  set_qureg(qr, 0, NQUBITS);

  // flip each qubit
  for (size_t i = 0; i < NQUBITS; ++i) {
    TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, paulix(&qr[i]));
  }

  // sv should now be in the all ones state
  getAmps(sv);
  TEST_ASSERT_EACH_EQUAL_DOUBLE(0.0, sv, 2 * (NAMPS-1));
  TEST_ASSERT_EQUAL_DOUBLE(1.0, creal(sv[NAMPS-1]));
  TEST_ASSERT_EQUAL_DOUBLE(0.0, cimag(sv[NAMPS-1]));

  TEST_ASSERT_EQUAL_INT(CQ_ERROR, paulix(NULL));
  
  return;
}

void test_pauliy(void) {
  complex double sv[NAMPS];

  set_qureg(qr, 0, NQUBITS);

  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, pauliy(&qr[0]));
  // qubit 0 now in state 1 with amp i
  getAmps(sv);
  TEST_ASSERT_EQUAL_DOUBLE(0.0, creal(sv[0]));
  TEST_ASSERT_EQUAL_DOUBLE(0.0, cimag(sv[0]));
  TEST_ASSERT_EQUAL_DOUBLE(0.0, creal(sv[1]));
  TEST_ASSERT_EQUAL_DOUBLE(1.0, cimag(sv[1]));
  TEST_ASSERT_EACH_EQUAL_DOUBLE(0.0, sv+2, 2 * (NAMPS - 2));

  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, pauliy(&qr[0]));
  // and back to all zeros
  getAmps(sv);
  TEST_ASSERT_EQUAL_DOUBLE(1.0, creal(sv[0]));
  TEST_ASSERT_EQUAL_DOUBLE(0.0, cimag(sv[0]));
  TEST_ASSERT_EACH_EQUAL_DOUBLE(0.0, sv+1, 2 * (NAMPS-1));

  TEST_ASSERT_EQUAL_INT(CQ_ERROR, pauliy(NULL));

  return;
}

void test_pauliz(void) {
  complex double sv[NAMPS];
  char msg[64];
  const double NORM_AMP = 1.0 / sqrt(NAMPS);

  init_plus_state();

  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, pauliz(&qr[NQUBITS-1]));
  // first half of sv should be as initialised, second half should be * -1
  getAmps(sv);
  for (size_t i = 0; i < NAMPS / 2; ++i) {
    sprintf(msg, "Real component: index = %lu", i);
    TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(NORM_AMP, creal(sv[i]), msg);
    sprintf(msg, "Imaginary component: index = %lu", i);
    TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(0.0, cimag(sv[i]), msg);
  }
  for (size_t i = NAMPS / 2; i < NAMPS; ++i) {
    sprintf(msg, "Real component: index = %lu", i);
    TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(-NORM_AMP, creal(sv[i]), msg);
    sprintf(msg, "Imaginary component: index = %lu", i);
    TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(0.0, cimag(sv[i]), msg);
  }

  TEST_ASSERT_EQUAL_INT(CQ_ERROR, pauliz(NULL));

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

void test_sqrtz(void) {
  complex double sv[NAMPS];
  char msg[64];
  const double NORM_AMP = 1.0 / sqrt(NAMPS);

  init_plus_state();

  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, sqrtz(&qr[NQUBITS-1]));
  // first half of sv should be as initialised, second half should be * i
  getAmps(sv);
  for (size_t i = 0; i < NAMPS / 2; ++i) {
    sprintf(msg, "Real component: index = %lu", i);
    TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(NORM_AMP, creal(sv[i]), msg);
    sprintf(msg, "Imaginary component: index = %lu", i);
    TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(0.0, cimag(sv[i]), msg);
  }
  for (size_t i = NAMPS / 2; i < NAMPS; ++i) {
    sprintf(msg, "Real component: index = %lu", i);
    TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(0.0, creal(sv[i]), msg);
    sprintf(msg, "Imaginary component: index = %lu", i);
    TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(NORM_AMP, cimag(sv[i]), msg);
  }

  TEST_ASSERT_EQUAL_INT(CQ_ERROR, sqrtz(NULL));

  return;
}

void test_sqrtzhc(void) {
  complex double sv[NAMPS];
  char msg[64];
  const double NORM_AMP = 1.0 / sqrt(NAMPS);

  init_plus_state();

  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, sqrtzhc(&qr[NQUBITS-1]));
  // first half of sv should be as initialised, second half should be * -i
  getAmps(sv);
  for (size_t i = 0; i < NAMPS / 2; ++i) {
    sprintf(msg, "Real component: index = %lu", i);
    TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(NORM_AMP, creal(sv[i]), msg);
    sprintf(msg, "Imaginary component: index = %lu", i);
    TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(0.0, cimag(sv[i]), msg);
  }
  for (size_t i = NAMPS / 2; i < NAMPS; ++i) {
    sprintf(msg, "Real component: index = %lu", i);
    TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(0.0, creal(sv[i]), msg);
    sprintf(msg, "Imaginary component: index = %lu", i);
    TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(-NORM_AMP, cimag(sv[i]), msg);
  }

  TEST_ASSERT_EQUAL_INT(CQ_ERROR, sqrtzhc(NULL));

  return;
}

void test_sqrts(void) {
  complex double sv[NAMPS];
  char msg[64];
  const double NORM_AMP = 1.0 / sqrt(NAMPS);
  const double root_S = 1.0 / sqrt(2);

  init_plus_state();

  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, sqrts(&qr[NQUBITS-1]));
  // first half of sv should be as initialised, second half should be * (1 + I)/sqrt(2)
  getAmps(sv);
  for (size_t i = 0; i < NAMPS / 2; ++i) {
    sprintf(msg, "Real component: index = %lu", i);
    TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(NORM_AMP, creal(sv[i]), msg);
    sprintf(msg, "Imaginary component: index = %lu", i);
    TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(0.0, cimag(sv[i]), msg);
  }
  for (size_t i = NAMPS / 2; i < NAMPS; ++i) {
    sprintf(msg, "Real component: index = %lu", i);
    TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(NORM_AMP * root_S, creal(sv[i]), msg);
    sprintf(msg, "Imaginary component: index = %lu", i);
    TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(NORM_AMP * root_S, cimag(sv[i]), msg);
  }

  TEST_ASSERT_EQUAL_INT(CQ_ERROR, sqrts(NULL));

  return;
}

void test_sqrtshc(void) {
  complex double sv[NAMPS];
  char msg[64];
  const double NORM_AMP = 1.0 / sqrt(NAMPS);
  const double root_S = 1.0 / sqrt(2);

  init_plus_state();

  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, sqrtshc(&qr[NQUBITS-1]));
  // first half of sv should be as initialised, second half should be * (1 - I)/sqrt(2)
  getAmps(sv);
  for (size_t i = 0; i < NAMPS / 2; ++i) {
    sprintf(msg, "Real component: index = %lu", i);
    TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(NORM_AMP, creal(sv[i]), msg);
    sprintf(msg, "Imaginary component: index = %lu", i);
    TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(0.0, cimag(sv[i]), msg);
  }
  for (size_t i = NAMPS / 2; i < NAMPS; ++i) {
    sprintf(msg, "Real component: index = %lu", i);
    TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(NORM_AMP * root_S, creal(sv[i]), msg);
    sprintf(msg, "Imaginary component: index = %lu", i);
    TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(-NORM_AMP * root_S, cimag(sv[i]), msg);
  }

  TEST_ASSERT_EQUAL_INT(CQ_ERROR, sqrtshc(NULL));

  return;
}

void test_sqrtx(void) {
  complex double sv[NAMPS];

  // init to all zero
  set_qureg(qr, 0, NQUBITS);

  // flip each qubit
  for (size_t i = 0; i < NQUBITS; ++i) {
    TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, sqrtx(&qr[i]));
    TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, sqrtx(&qr[i]));
  }

  // sv should now be in the all ones state
  getAmps(sv);
  TEST_ASSERT_EACH_EQUAL_DOUBLE(0.0, sv, 2 * (NAMPS-1));
  TEST_ASSERT_EQUAL_DOUBLE(1.0, creal(sv[NAMPS-1]));
  TEST_ASSERT_EQUAL_DOUBLE(0.0, cimag(sv[NAMPS-1]));

  TEST_ASSERT_EQUAL_INT(CQ_ERROR, sqrtx(NULL));
  
  return;
}

void test_rotx(void) {
  size_t i;
  complex double sv[NAMPS];
  char msg[64];
  double theta;
  const double NORM_AMP = 1.0 / sqrt(NAMPS);

  init_plus_state();

  // identity
  theta = 0;
  for (i = 0; i < NQUBITS; ++i) {
    TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, rotx(&qr[i], theta));
  }

  getAmps(sv);
  for (i = 0; i < NAMPS; ++i) {
    sprintf(msg, "Real component: index = %lu", i);
    TEST_ASSERT_DOUBLE_WITHIN_MESSAGE(DELTA, NORM_AMP, creal(sv[i]), msg);
    sprintf(msg, "Imaginary component: index = %lu", i);
    TEST_ASSERT_DOUBLE_WITHIN_MESSAGE(DELTA, 0.0, cimag(sv[i]), msg);
  }

  // { {0, -I}, {-I, 0} }
  theta = M_PI;
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, rotx(&qr[0], theta));

  getAmps(sv);
  for (i = 0; i < NAMPS; ++i) {
    sprintf(msg, "Real component: index = %lu", i);
    TEST_ASSERT_DOUBLE_WITHIN_MESSAGE(DELTA, 0.0, creal(sv[i]), msg);
    sprintf(msg, "Imaginary component: index = %lu", i);
    TEST_ASSERT_DOUBLE_WITHIN_MESSAGE(DELTA, -NORM_AMP, cimag(sv[i]), msg);
  }

  TEST_ASSERT_EQUAL_INT(CQ_ERROR, rotx(NULL, theta));

  return;
}

void test_roty(void) {
  size_t i;
  complex double sv[NAMPS];
  char msg[64];
  double theta;
  const double NORM_AMP = 1.0 / sqrt(NAMPS);

  init_plus_state();

  // identity
  theta = 0;
  for (i = 0; i < NQUBITS; ++i) {
    TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, roty(&qr[i], theta));
  }

  getAmps(sv);
  for (i = 0; i < NAMPS; ++i) {
    sprintf(msg, "Real component: index = %lu", i);
    TEST_ASSERT_DOUBLE_WITHIN_MESSAGE(DELTA, NORM_AMP, creal(sv[i]), msg);
    sprintf(msg, "Imaginary component: index = %lu", i);
    TEST_ASSERT_DOUBLE_WITHIN_MESSAGE(DELTA, 0.0, cimag(sv[i]), msg);
  }

  // { {0, 1}, {-1, 0} }
  theta = M_PI;
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, roty(&qr[0], theta));

  getAmps(sv);
  for (i = 0; i < NAMPS; i += 2) {
    sprintf(msg, "Real component: index = %lu", i);
    TEST_ASSERT_DOUBLE_WITHIN_MESSAGE(DELTA, -NORM_AMP, creal(sv[i]), msg);
    sprintf(msg, "Imaginary component: index = %lu", i);
    TEST_ASSERT_DOUBLE_WITHIN_MESSAGE(DELTA, 0.0, cimag(sv[i]), msg);
  }
  for (i = 1; i < NAMPS; i += 2) {
    sprintf(msg, "Real component: index = %lu", i);
    TEST_ASSERT_DOUBLE_WITHIN_MESSAGE(DELTA, NORM_AMP, creal(sv[i]), msg);
    sprintf(msg, "Imaginary component: index = %lu", i);
    TEST_ASSERT_DOUBLE_WITHIN_MESSAGE(DELTA, 0.0, cimag(sv[i]), msg);
  }

  TEST_ASSERT_EQUAL_INT(CQ_ERROR, roty(NULL, theta));

  return;
}

void test_rotz(void) {
  size_t i;
  complex double sv[NAMPS];
  char msg[64];
  double theta;
  const double NORM_AMP = 1.0 / sqrt(NAMPS);

  init_plus_state();

  // identity
  theta = 0;
  for (i = 0; i < NQUBITS; ++i) {
    TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, roty(&qr[i], theta));
  }

  getAmps(sv);
  for (i = 0; i < NAMPS; ++i) {
    sprintf(msg, "Real component: index = %lu", i);
    TEST_ASSERT_DOUBLE_WITHIN_MESSAGE(DELTA, NORM_AMP, creal(sv[i]), msg);
    sprintf(msg, "Imaginary component: index = %lu", i);
    TEST_ASSERT_DOUBLE_WITHIN_MESSAGE(DELTA, 0.0, cimag(sv[i]), msg);
  }

  // negative identity
  theta = 2 * M_PI;
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, rotz(&qr[0], theta));

  getAmps(sv);
  for (i = 0; i < NAMPS; ++i) {
    sprintf(msg, "Real component: index = %lu", i);
    TEST_ASSERT_DOUBLE_WITHIN_MESSAGE(DELTA, -NORM_AMP, creal(sv[i]), msg);
    sprintf(msg, "Imaginary component: index = %lu", i);
    TEST_ASSERT_DOUBLE_WITHIN_MESSAGE(DELTA, 0.0, cimag(sv[i]), msg);
  }

  TEST_ASSERT_EQUAL_INT(CQ_ERROR, rotz(NULL, theta));

  return;
}

void test_cpaulix(void) {
  size_t tgt, ctrl;
  complex double sv[NAMPS];
  cq_status status;

  // init qureg to all zeros
  set_qureg(qr, 0, NQUBITS);

  getAmps(sv);
  TEST_ASSERT_EQUAL_DOUBLE(1.0, creal(sv[0]));
  TEST_ASSERT_EQUAL_DOUBLE(0.0, cimag(sv[0]));
  TEST_ASSERT_EACH_EQUAL_DOUBLE(0.0, &sv[1], 2 * (NAMPS-1));

  // should be all identities!
  for (ctrl = 0; ctrl < NQUBITS; ++ctrl) {
    for (tgt = 0; tgt < NQUBITS; ++tgt) {
      if (ctrl == tgt) {
        status = CQ_ERROR;
      } else {
        status = CQ_SUCCESS;
      }

      TEST_ASSERT_EQUAL_INT(status, cpaulix(&qr[ctrl], &qr[tgt]));

      getAmps(sv);
      TEST_ASSERT_EQUAL_DOUBLE(1.0, creal(sv[0]));
      TEST_ASSERT_EQUAL_DOUBLE(0.0, cimag(sv[0]));
      TEST_ASSERT_EACH_EQUAL_DOUBLE(0.0, &sv[1], 2 * (NAMPS-1));
    }
  }

  // set 0th qubit to 1
  set_qureg(qr, 1, NQUBITS);
  getAmps(sv);
  TEST_ASSERT_EACH_EQUAL_DOUBLE(0.0, sv, 2);
  TEST_ASSERT_EQUAL_DOUBLE(1.0, creal(sv[1]));
  TEST_ASSERT_EQUAL_DOUBLE(0.0, cimag(sv[1]));
  TEST_ASSERT_EACH_EQUAL_DOUBLE(0.0, &sv[2], 2 * (NAMPS-2));

  // flip each other qubit using 0 as ctrl
  ctrl = 0;
  for (tgt = 1; tgt < NQUBITS; ++tgt) {
    TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, cpaulix(&qr[ctrl], &qr[tgt]));
  }
  getAmps(sv);
  TEST_ASSERT_EACH_EQUAL_DOUBLE(0.0, sv, 2 * (NAMPS-1));
  TEST_ASSERT_EQUAL_DOUBLE(1.0, creal(sv[NAMPS-1]));
  TEST_ASSERT_EQUAL_DOUBLE(0.0, cimag(sv[NAMPS-1]));

  TEST_ASSERT_EQUAL_INT(CQ_ERROR, cpaulix(NULL, NULL));
  TEST_ASSERT_EQUAL_INT(CQ_ERROR, cpaulix(NULL, &qr[0]));
  TEST_ASSERT_EQUAL_INT(CQ_ERROR, cpaulix(&qr[0], NULL));

  return;
}

void test_cpauliy(void) {
  complex double sv[NAMPS];
  char msg[64];

  set_qureg(qr, 1, NQUBITS);

  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, cpauliy(&qr[0], &qr[NQUBITS-1]));
  // qubit 0 in state 1 and qubit N-1 now in state 1 with amp i 
  getAmps(sv);
  for (size_t i = 0; i < NAMPS; ++i) {
    if (i == (NAMPS/2) + 1) {
      sprintf(msg, "Real component: index = %lu", i);
      TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(0.0, creal(sv[i]), msg);
      sprintf(msg, "Imaginary component: index = %lu", i);
      TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(1.0, cimag(sv[i]), msg);
    } else {
      TEST_ASSERT_EACH_EQUAL_DOUBLE(0.0, sv + i, 2);
    }
  }

  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, cpauliy(&qr[0], &qr[NQUBITS-1]));
  // and back to state 1
  getAmps(sv);
  TEST_ASSERT_EQUAL_DOUBLE(0.0, creal(sv[0]));
  TEST_ASSERT_EQUAL_DOUBLE(0.0, cimag(sv[0]));
  TEST_ASSERT_EQUAL_DOUBLE(1.0, creal(sv[1]));
  TEST_ASSERT_EQUAL_DOUBLE(0.0, cimag(sv[1]));
  TEST_ASSERT_EACH_EQUAL_DOUBLE(0.0, sv+2, 2 * (NAMPS-2));

  for (size_t ctrl = 0; ctrl < NQUBITS; ++ctrl) {
    for (size_t tgt = 0; tgt < NQUBITS; ++tgt) {
      if (ctrl == tgt) {
        TEST_ASSERT_EQUAL_INT(CQ_ERROR, cpauliy(&qr[ctrl], &qr[tgt]));
      } else {
        TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, cpauliy(&qr[ctrl], &qr[tgt]));
      }
    }
  }

  TEST_ASSERT_EQUAL_INT(CQ_ERROR, cpauliy(NULL, NULL));
  TEST_ASSERT_EQUAL_INT(CQ_ERROR, cpauliy(&qr[0], NULL));
  TEST_ASSERT_EQUAL_INT(CQ_ERROR, cpauliy(NULL, &qr[0]));

  return;
}

void test_cpauliz(void) {
  complex double sv[NAMPS];
  size_t ctrl, tgt;

  // initialise to zeros
  set_qureg(qr, 0, NQUBITS);
  getAmps(sv);
  TEST_ASSERT_EQUAL_DOUBLE(1.0, creal(sv[0]));
  TEST_ASSERT_EQUAL_DOUBLE(0.0, cimag(sv[0]));
  TEST_ASSERT_EACH_EQUAL_DOUBLE(0.0, sv+1, 2 * (NAMPS-1));

  // should be all identities!
  for (ctrl = 0; ctrl < NQUBITS; ++ctrl) {
    for (tgt = 0; tgt < NQUBITS; ++tgt) {
      if (ctrl == tgt) {
        TEST_ASSERT_EQUAL_INT(CQ_ERROR, cpauliz(&qr[ctrl], &qr[tgt]));
      } else {
        TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, cpauliz(&qr[ctrl], &qr[tgt]));
        
        getAmps(sv);
        TEST_ASSERT_EQUAL_DOUBLE(1.0, creal(sv[0]));
        TEST_ASSERT_EQUAL_DOUBLE(0.0, cimag(sv[0]));
        TEST_ASSERT_EACH_EQUAL_DOUBLE(0.0, sv+1, 2 * (NAMPS-1));
      }
    }
  }

  // set qubit 0 to 1, and qubit N-1 to +
  set_qureg(qr, 1, NQUBITS);
  hadamard(&qr[NQUBITS-1]);
  getAmps(sv);
  const double NORM_FAC = 1.0 / sqrt(2);
  TEST_ASSERT_EQUAL_DOUBLE(NORM_FAC, creal(sv[1]));
  TEST_ASSERT_EQUAL_DOUBLE(NORM_FAC, creal(sv[NAMPS/2 + 1]));

  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, cpauliz(&qr[0], &qr[NQUBITS-1]));
  getAmps(sv);
  TEST_ASSERT_EQUAL_DOUBLE(NORM_FAC, creal(sv[1]));
  TEST_ASSERT_EQUAL_DOUBLE(-NORM_FAC, creal(sv[NAMPS/2 + 1]));

  TEST_ASSERT_EQUAL_INT(CQ_ERROR, cpauliz(NULL, NULL));
  TEST_ASSERT_EQUAL_INT(CQ_ERROR, cpauliz(&qr[0], NULL));
  TEST_ASSERT_EQUAL_INT(CQ_ERROR, cpauliz(NULL, &qr[0]));

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

void test_crotx(void) {
  complex double sv[NAMPS];
  char msg[64];
  double theta;
  const double NORM_AMP = 1.0 / sqrt(2);

  set_qureg(qr, 0, NQUBITS);

  // identity
  theta = 0;
  for (size_t ctrl = 0; ctrl < NQUBITS; ++ctrl) {
    for (size_t tgt = 0; tgt < NQUBITS; ++tgt) {
      if (ctrl == tgt) {
        TEST_ASSERT_EQUAL_INT(CQ_ERROR, crotx(&qr[ctrl], &qr[tgt], theta));
      } else {
        TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, crotx(&qr[ctrl], &qr[tgt], theta));
      
        getAmps(sv);
        TEST_ASSERT_EQUAL_DOUBLE(1.0, creal(sv[0]));
        TEST_ASSERT_EQUAL_DOUBLE(0.0, cimag(sv[0]));
        TEST_ASSERT_EACH_EQUAL_DOUBLE(0.0, sv+1, 2 * (NAMPS-1));
      }
    }
  }

  // set qubit 0 to |1>
  set_qureg(qr, 1, NQUBITS);
  for (size_t tgt = 1; tgt < NQUBITS; ++tgt) {
    TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, crotx(&qr[0], &qr[tgt], theta));
  
    getAmps(sv);
    TEST_ASSERT_EACH_EQUAL_DOUBLE(0.0, sv, 2);
    TEST_ASSERT_EQUAL_DOUBLE(1.0, creal(sv[1]));
    TEST_ASSERT_EQUAL_DOUBLE(0.0, cimag(sv[1]));
    TEST_ASSERT_EACH_EQUAL_DOUBLE(0.0, sv+2, 2 * (NAMPS-2));
  }

  // set qubit N-1 to |+>
  hadamard(&qr[NQUBITS-1]);

  // { {0, -I}, {-I, 0} }
  theta = M_PI;
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, crotx(&qr[0], &qr[NQUBITS-1], theta));

  getAmps(sv);
  for (size_t i = 0; i < NAMPS; ++i) {
    if (i == 1 || i == NAMPS/2 + 1) {
      sprintf(msg, "Real component: index = %lu", i);
      TEST_ASSERT_DOUBLE_WITHIN_MESSAGE(DELTA, 0.0, creal(sv[i]), msg);
      sprintf(msg, "Imaginary component: index = %lu", i);
      TEST_ASSERT_DOUBLE_WITHIN_MESSAGE(DELTA, -NORM_AMP, cimag(sv[i]), msg);
    } else {
      sprintf(msg, "Real component: index = %lu", i);
      TEST_ASSERT_DOUBLE_WITHIN_MESSAGE(DELTA, 0.0, creal(sv[i]), msg);
      sprintf(msg, "Imaginary component: index = %lu", i);
      TEST_ASSERT_DOUBLE_WITHIN_MESSAGE(DELTA, 0.0, cimag(sv[i]), msg);
    }
  }

  TEST_ASSERT_EQUAL_INT(CQ_ERROR, crotx(NULL, NULL, theta));
  TEST_ASSERT_EQUAL_INT(CQ_ERROR, crotx(&qr[0], NULL, theta));
  TEST_ASSERT_EQUAL_INT(CQ_ERROR, crotx(NULL, &qr[0], theta));

  return;
}

void test_croty(void) {
  complex double sv[NAMPS];
  char msg[64];
  double theta;

  set_qureg(qr, 0, NQUBITS);

  // identity
  theta = 0;
  for (size_t ctrl = 0; ctrl < NQUBITS; ++ctrl) {
    for (size_t tgt = 0; tgt < NQUBITS; ++tgt) {
      if (tgt == ctrl) {
        TEST_ASSERT_EQUAL_INT(CQ_ERROR, croty(&qr[ctrl], &qr[tgt], 0.0));
      } else {
        TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, croty(&qr[ctrl], &qr[tgt], 0.0));

        getAmps(sv);
        TEST_ASSERT_EQUAL_DOUBLE(1.0, creal(sv[0]));
        TEST_ASSERT_EQUAL_DOUBLE(0.0, cimag(sv[0]));
        TEST_ASSERT_EACH_EQUAL_DOUBLE(0.0, sv+1, 2 * (NAMPS-1));
      }
    }
  }

  // set qubit 0 to |1>
  set_qureg(qr, 1, NQUBITS);
  for (size_t tgt = 1; tgt < NQUBITS; ++tgt) {
    TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, croty(&qr[0], &qr[tgt], theta));
    
    getAmps(sv);
    TEST_ASSERT_EACH_EQUAL_DOUBLE(0.0, sv, 2);
    TEST_ASSERT_EQUAL_DOUBLE(1.0, creal(sv[1]));
    TEST_ASSERT_EQUAL_DOUBLE(0.0, cimag(sv[1]));
    TEST_ASSERT_EACH_EQUAL_DOUBLE(0.0, sv+2, 2 * (NAMPS-2));
  }

  // set qubit N-1 to |+>
  hadamard(&qr[NQUBITS-1]);

  // { {0, 1}, {-1, 0} }
  theta = M_PI;
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, croty(&qr[0], &qr[NQUBITS-1], theta));

  const double NORM_FAC = 1.0 / sqrt(2);
  getAmps(sv);
  for (size_t i = 0; i < NAMPS; i += 2) {
    if (i == 1) {
      TEST_ASSERT_DOUBLE_WITHIN(DELTA, NORM_FAC, creal(sv[i]));
      TEST_ASSERT_DOUBLE_WITHIN(DELTA, 0.0, cimag(sv[i]));
    } else if (i == NAMPS / 2 + 1) {
      TEST_ASSERT_DOUBLE_WITHIN(DELTA, -NORM_FAC, creal(sv[i]));
      TEST_ASSERT_DOUBLE_WITHIN(DELTA, 0.0, cimag(sv[i]));
    } else {
      sprintf(msg, "Real component: index = %lu", i);
      TEST_ASSERT_DOUBLE_WITHIN_MESSAGE(DELTA, 0.0, creal(sv[i]), msg);
      sprintf(msg, "Imaginary component: index = %lu", i);
      TEST_ASSERT_DOUBLE_WITHIN_MESSAGE(DELTA, 0.0, cimag(sv[i]), msg);
    }
  }

  TEST_ASSERT_EQUAL_INT(CQ_ERROR, croty(NULL, NULL, theta));
  TEST_ASSERT_EQUAL_INT(CQ_ERROR, croty(&qr[0], NULL, theta));
  TEST_ASSERT_EQUAL_INT(CQ_ERROR, croty(NULL, &qr[0], theta));
 
  return;
}

void test_crotz(void) {
  complex double sv[NAMPS];
  char msg[64];
  double theta;

  // identities
  theta = 0;
  set_qureg(qr, 0, NQUBITS);
  for (size_t ctrl = 0; ctrl < NQUBITS; ++ctrl) {
    for (size_t tgt = 0; tgt < NQUBITS; ++tgt) {
      if (ctrl == tgt) {
        TEST_ASSERT_EQUAL_INT(CQ_ERROR, crotz(&qr[ctrl], &qr[tgt], theta));
      } else {
        TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, crotz(&qr[ctrl], &qr[tgt], theta));

        getAmps(sv);
        TEST_ASSERT_EQUAL_DOUBLE(1.0, creal(sv[0]));
        TEST_ASSERT_EQUAL_DOUBLE(0.0, cimag(sv[0]));
        TEST_ASSERT_EACH_EQUAL_DOUBLE(0.0, sv+1, 2 * (NAMPS-1));
      }
    }
  }

  // set qubit 0 to |1>
  set_qureg(qr, 1, NQUBITS);
  for (size_t tgt = 1; tgt < NQUBITS; ++tgt) {
    TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, crotz(&qr[0], &qr[tgt], theta));
    
    getAmps(sv);
    TEST_ASSERT_EACH_EQUAL_DOUBLE(0.0, sv, 2);
    TEST_ASSERT_EQUAL_DOUBLE(1.0, creal(sv[1]));
    TEST_ASSERT_EQUAL_DOUBLE(0.0, cimag(sv[1]));
    TEST_ASSERT_EACH_EQUAL_DOUBLE(0.0, sv+2, 2 * (NAMPS-2));
  }

  // set qubit N-1 to |+>
  hadamard(&qr[NQUBITS-1]);

  // negative identity
  theta = 2 * M_PI;
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, crotz(&qr[0], &qr[NQUBITS-1], theta));

  const double NORM_FAC = 1.0 / sqrt(2);
  getAmps(sv);
  for (size_t i = 0; i < NAMPS; ++i) {
    if (i == 1 || i == NAMPS / 2 + 1) {
      TEST_ASSERT_DOUBLE_WITHIN(DELTA, -NORM_FAC, creal(sv[i]));
      TEST_ASSERT_DOUBLE_WITHIN(DELTA, 0.0, cimag(sv[i]));
    } else {
      sprintf(msg, "Real component: index = %lu", i);
      TEST_ASSERT_DOUBLE_WITHIN_MESSAGE(DELTA, 0.0, creal(sv[i]), msg);
      sprintf(msg, "Imaginary component: index = %lu", i);
      TEST_ASSERT_DOUBLE_WITHIN_MESSAGE(DELTA, 0.0, cimag(sv[i]), msg);
    }
  }

  TEST_ASSERT_EQUAL_INT(CQ_ERROR, crotz(NULL, NULL, theta));
  TEST_ASSERT_EQUAL_INT(CQ_ERROR, crotz(&qr[0], NULL, theta));
  TEST_ASSERT_EQUAL_INT(CQ_ERROR, crotz(NULL, &qr[0], theta));

  return;
}

void test_chadamard(void) {
  complex double sv[NAMPS];

  set_qureg(qr, 0, NQUBITS);

  // Identities
  for (size_t ctrl = 0; ctrl < NQUBITS; ++ctrl) {
    for (size_t tgt = 0; tgt < NQUBITS; ++tgt) {
      if (ctrl == tgt) {
        TEST_ASSERT_EQUAL_INT(CQ_ERROR, chadamard(&qr[ctrl], &qr[tgt]));
      } else {
        TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, chadamard(&qr[ctrl], &qr[tgt]));

        getAmps(sv);
        TEST_ASSERT_EQUAL_DOUBLE(1.0, creal(sv[0]));
        TEST_ASSERT_EQUAL_DOUBLE(0.0, cimag(sv[0]));
        TEST_ASSERT_EACH_EQUAL_DOUBLE(0.0, sv+1, 2 * (NAMPS-1));
      }
    }
  }

  set_qureg(qr, 1, NQUBITS);

  for (size_t tgt = 1; tgt < NQUBITS; ++tgt) {
    TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, chadamard(&qr[0], &qr[tgt]));
  }

  hadamard(&qr[0]);

  // Minus sign where qubit 0 in |1> because of the hadamard
  const double NORM_AMP = 1.0 / sqrt(NAMPS);
  getAmps(sv);
  for (size_t i = 0; i < NAMPS; i += 2) {
    TEST_ASSERT_EQUAL_DOUBLE(NORM_AMP, creal(sv[i]));
    TEST_ASSERT_EQUAL_DOUBLE(0.0, cimag(sv[i]));
  }
  for (size_t i = 1; i < NAMPS; i += 2) {
    TEST_ASSERT_EQUAL_DOUBLE(-NORM_AMP, creal(sv[i]));
    TEST_ASSERT_EQUAL_DOUBLE(0.0, cimag(sv[i]));
  }

  TEST_ASSERT_EQUAL_INT(CQ_ERROR, chadamard(NULL, NULL));
  TEST_ASSERT_EQUAL_INT(CQ_ERROR, chadamard(&qr[0], NULL));
  TEST_ASSERT_EQUAL_INT(CQ_ERROR, chadamard(NULL, &qr[0]));

  return;
}

void test_cunitary(void) {
  const double NORM_AMP = 1.0 / sqrt(NAMPS);
  complex double sv[NAMPS];
  char msg[64];
  double theta, phi, lambda;
  size_t ctrl, tgt;

  // Identities
  theta = 0.0;
  phi = 0.0;
  lambda = 0.0;

  set_qureg(qr, 0, NQUBITS);
  for (ctrl = 0; ctrl < NQUBITS; ++ctrl) {
    for (tgt = 0; tgt < NQUBITS; ++tgt) {
      if (ctrl == tgt) {
        TEST_ASSERT_EQUAL_INT(CQ_ERROR, cunitary(&qr[ctrl], &qr[tgt], theta, phi, lambda));
      } else {
        TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, cunitary(&qr[ctrl], &qr[tgt], theta, phi, lambda));

        getAmps(sv);
        TEST_ASSERT_EQUAL_DOUBLE(1.0, creal(sv[0]));
        TEST_ASSERT_EQUAL_DOUBLE(0.0, cimag(sv[0]));
        TEST_ASSERT_EACH_EQUAL_DOUBLE(0.0, sv+1, 2 * (NAMPS-1));
      }
    }
  }

  set_qureg(qr, 1, NQUBITS);
  for (tgt = 1; tgt < NQUBITS; ++tgt) {
    TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, cunitary(&qr[0], &qr[tgt], theta, phi, lambda));

    getAmps(sv);
    TEST_ASSERT_EACH_EQUAL_DOUBLE(0.0, sv, 2);
    TEST_ASSERT_EQUAL_DOUBLE(1.0, creal(sv[1]));
    TEST_ASSERT_EQUAL_DOUBLE(0.0, cimag(sv[1]));
    TEST_ASSERT_EACH_EQUAL_DOUBLE(0.0, sv+2, 2 * (NAMPS-2));
  }


  // NOT
  theta = M_PI;
  phi = 3 * M_PI / 2;
  lambda = M_PI / 2;

  // ctrl = |0>
  set_qureg(qr, 0, NQUBITS);
  for (ctrl = 0; ctrl < NQUBITS; ++ctrl) {
    for (tgt = 0; tgt < NQUBITS; ++tgt) {
      if (ctrl == tgt) {
        TEST_ASSERT_EQUAL_INT(CQ_ERROR, cunitary(&qr[ctrl], &qr[tgt], theta, phi, lambda));
      } else {
        TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, cunitary(&qr[ctrl], &qr[tgt], theta, phi, lambda));

        getAmps(sv);
        TEST_ASSERT_EQUAL_DOUBLE(1.0, creal(sv[0]));
        TEST_ASSERT_EQUAL_DOUBLE(0.0, cimag(sv[0]));
        TEST_ASSERT_EACH_EQUAL_DOUBLE(0.0, sv+1, 2 * (NAMPS-1));
      }
    }
  }

  // ctrl = |1>

  set_qureg(qr, 1, NQUBITS);
  for (tgt = 1; tgt < NQUBITS; ++tgt) {
    TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, cunitary(&qr[0], &qr[tgt], theta, phi, lambda));
  }

  
  double expected[2 * (NAMPS-1)];
  for (size_t i = 0; i < 2 * (NAMPS-1); ++i) expected[i] = 0.0;

  getAmps(sv);
  TEST_ASSERT_DOUBLE_ARRAY_WITHIN(DELTA, expected, sv, 2 * (NAMPS-1));
  TEST_ASSERT_DOUBLE_WITHIN(DELTA, 1.0, creal(sv[NAMPS-1]));
  TEST_ASSERT_DOUBLE_WITHIN(DELTA, 0.0, cimag(sv[NAMPS-1]));

  TEST_ASSERT_EQUAL_INT(CQ_ERROR, cunitary(NULL, NULL, theta, phi, lambda));
  TEST_ASSERT_EQUAL_INT(CQ_ERROR, cunitary(&qr[0], NULL, theta, phi, lambda));
  TEST_ASSERT_EQUAL_INT(CQ_ERROR, cunitary(NULL, &qr[0], theta, phi, lambda));

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

void test_ccpaulix(void) {
  complex double sv[NAMPS];
  size_t ctrl_A, ctrl_B, target;

  // identities

  // everything |0>
  set_qureg(qr, 0, NQUBITS);

  for (ctrl_A = 0; ctrl_A < NQUBITS; ++ctrl_A) {
    for (ctrl_B = 0; ctrl_B < NQUBITS; ++ctrl_B) {
      for (target = 0; target < NQUBITS; ++target) {
        if (ctrl_A == ctrl_B || ctrl_A == target || ctrl_B == target) {
          TEST_ASSERT_EQUAL_INT(CQ_ERROR, ccpaulix(&qr[ctrl_A], &qr[ctrl_B], &qr[target]));
        } else {
          TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, ccpaulix(&qr[ctrl_A], &qr[ctrl_B], &qr[target]));

          getAmps(sv);
          TEST_ASSERT_EQUAL_DOUBLE(1.0, creal(sv[0]));
          TEST_ASSERT_EQUAL_DOUBLE(0.0, cimag(sv[0]));
          TEST_ASSERT_EACH_EQUAL_DOUBLE(0.0, sv+1, 2 * (NAMPS-1));
        }
      }
    }
  }

  // ctrl A is |1> ctrl B is |0>
  set_qureg(qr, 1, NQUBITS);
  for (ctrl_B = 1; ctrl_B < NQUBITS; ++ctrl_B) {
    for (target = 1; target < NQUBITS; ++target) {
      if (ctrl_B == target) {
        TEST_ASSERT_EQUAL_INT(CQ_ERROR, ccpaulix(&qr[0], &qr[ctrl_B], &qr[target]));
      } else {
        TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, ccpaulix(&qr[0], &qr[ctrl_B], &qr[target]));

        getAmps(sv);
        TEST_ASSERT_EACH_EQUAL_DOUBLE(0.0, sv, 2);
        TEST_ASSERT_EQUAL_DOUBLE(1.0, creal(sv[1]));
        TEST_ASSERT_EQUAL_DOUBLE(0.0, cimag(sv[1]));
        TEST_ASSERT_EACH_EQUAL_DOUBLE(0.0, sv+2, 2 * (NAMPS-2));
      }
    }
  }

  // ctrl B is |1> ctrl A is |0>
  for (ctrl_A = 1; ctrl_A < NQUBITS; ++ctrl_A) {
    for (target = 1; target < NQUBITS; ++target) {
      if (ctrl_A == target) {
        TEST_ASSERT_EQUAL_INT(CQ_ERROR, ccpaulix(&qr[ctrl_A], &qr[0], &qr[target]));
      } else {
        TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, ccpaulix(&qr[ctrl_A], &qr[0], &qr[target]));

        getAmps(sv);
        TEST_ASSERT_EACH_EQUAL_DOUBLE(0.0, sv, 2);
        TEST_ASSERT_EQUAL_DOUBLE(1.0, creal(sv[1]));
        TEST_ASSERT_EQUAL_DOUBLE(0.0, cimag(sv[1]));
        TEST_ASSERT_EACH_EQUAL_DOUBLE(0.0, sv+2, 2 * (NAMPS-2));
      }
    }
  }

  // first and last qubit in |1>
  set_qureg(qr, NAMPS / 2 + 1, NQUBITS);

  // set all remaining qubits to |1>
  for (target = 1; target < NQUBITS - 1; ++target) {
    TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, ccpaulix(&qr[0], &qr[NQUBITS-1], &qr[target]));
  }

  getAmps(sv);
  TEST_ASSERT_EACH_EQUAL_DOUBLE(0.0, sv, 2 * (NAMPS-1));
  TEST_ASSERT_EQUAL_DOUBLE(1.0, creal(sv[NAMPS-1]));
  TEST_ASSERT_EQUAL_DOUBLE(0.0, cimag(sv[NAMPS-1]));

  TEST_ASSERT_EQUAL_INT(CQ_ERROR, ccpaulix(NULL, NULL, NULL));
  TEST_ASSERT_EQUAL_INT(CQ_ERROR, ccpaulix(&qr[0], &qr[1], NULL));
  TEST_ASSERT_EQUAL_INT(CQ_ERROR, ccpaulix(&qr[0], NULL, &qr[1]));
  TEST_ASSERT_EQUAL_INT(CQ_ERROR, ccpaulix(NULL, &qr[0], &qr[1]));
  TEST_ASSERT_EQUAL_INT(CQ_ERROR, ccpaulix(&qr[0], NULL, NULL));
  TEST_ASSERT_EQUAL_INT(CQ_ERROR, ccpaulix(NULL, &qr[0], NULL));
  TEST_ASSERT_EQUAL_INT(CQ_ERROR, ccpaulix(NULL, NULL, &qr[0]));

  return;
}

void test_cswap(void) {
  complex double sv[NAMPS];
  size_t ctrl, a, b;
  char msg[64];

  // Identities
  set_qureg(qr, 1, NQUBITS);

  // we fix a to 0 (which is in |1>) as otherwise 
  // errors would be undetectable
  a = 0;
  for (ctrl = 1; ctrl < NQUBITS; ++ctrl) {
    for (b = 1; b < NQUBITS; ++b) {
      if (ctrl == b) {
        TEST_ASSERT_EQUAL_INT(CQ_ERROR, cswap(&qr[ctrl], &qr[a], &qr[b]));
      } else {
        TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, cswap(&qr[ctrl], &qr[a], &qr[b]));

        getAmps(sv);
        TEST_ASSERT_EACH_EQUAL_DOUBLE(0.0, sv, 2);
        TEST_ASSERT_EQUAL_DOUBLE(1.0, creal(sv[1]));
        TEST_ASSERT_EQUAL_DOUBLE(0.0, cimag(sv[1]));
        TEST_ASSERT_EACH_EQUAL_DOUBLE(0.0, sv+2, 2 * (NAMPS - 2));
      }
    }
  }

  b = 0;
  for (ctrl = 1; ctrl < NQUBITS; ++ctrl) {
    for (a = 1; a < NQUBITS; ++a) {
      if (ctrl == a) {
        TEST_ASSERT_EQUAL_INT(CQ_ERROR, cswap(&qr[ctrl], &qr[a], &qr[b]));
      } else {
        TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, cswap(&qr[ctrl], &qr[a], &qr[b]));

        getAmps(sv);
        TEST_ASSERT_EACH_EQUAL_DOUBLE(0.0, sv, 2);
        TEST_ASSERT_EQUAL_DOUBLE(1.0, creal(sv[1]));
        TEST_ASSERT_EQUAL_DOUBLE(0.0, cimag(sv[1]));
        TEST_ASSERT_EACH_EQUAL_DOUBLE(0.0, sv+2, 2 * (NAMPS - 2));
      }
    }
  }

  // now set to state 3, and move the msb up by one each time
  set_qureg(qr, 3, NQUBITS);
  ctrl = 0;
  a = 1;
  for (b = 2; b < NQUBITS; ++b) {
    TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, cswap(&qr[0], &qr[a], &qr[b]));

    getAmps(sv);
    for (size_t i = 0; i < NAMPS; ++i) {
      sprintf(msg, "i = %lu, a = %lu, b = %lu", i, a, b);
      if (check_bit(i, 0) && check_bit(i, b) && pop_count(i) == 2) {
        TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(1.0, creal(sv[i]), msg);
      } else {
        TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(0.0, creal(sv[i]), msg);
      }
      TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(0.0, cimag(sv[i]), msg);
    }
    
    a = b;
  }

  TEST_ASSERT_EQUAL_INT(CQ_ERROR, cswap(&qr[0], &qr[1], &qr[1]));
  TEST_ASSERT_EQUAL_INT(CQ_ERROR, cswap(NULL, NULL, NULL));
  TEST_ASSERT_EQUAL_INT(CQ_ERROR, cswap(&qr[0], &qr[1], NULL));
  TEST_ASSERT_EQUAL_INT(CQ_ERROR, cswap(&qr[0], NULL, &qr[1]));
  TEST_ASSERT_EQUAL_INT(CQ_ERROR, cswap(NULL, &qr[0], &qr[1]));
  TEST_ASSERT_EQUAL_INT(CQ_ERROR, cswap(&qr[0], NULL, NULL));
  TEST_ASSERT_EQUAL_INT(CQ_ERROR, cswap(NULL, &qr[0], NULL));
  TEST_ASSERT_EQUAL_INT(CQ_ERROR, cswap(NULL, NULL, &qr[0]));

  return;
}