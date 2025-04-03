#include <complex.h>
#include <math.h>
#include "unity.h"
#include "datatypes.h"
#include "host_ops.h"
#include "device_ops.h"
#include "src/device/resources.h"
#include "qasm_gates.h"
#include "test_qasm_gates.h"

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

void test_hadamard(void) {
  const double EXPECTED = 1.0 / sqrt(NAMPS);
  char msg[64];
  complex double sv[NAMPS];
  
  set_qureg(qr, 0, NQUBITS);
  for (size_t i = 0; i < NQUBITS; ++i) {
    TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, hadamard(&qr[i]));
  }

  getQuregAmps(sv, qregistry.registers[qr[0].registry_index], 0, NAMPS);
  for (size_t i = 0; i < NAMPS; ++i) {
    sprintf(msg, "Real component: index = %lu", i);
    TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(EXPECTED, creal(sv[i]), msg);

    sprintf(msg, "Imaginary component: index = %lu", i);
    TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(0.0, cimag(sv[i]), msg);
  }

  for (size_t i = 0; i < NQUBITS; ++i) {
    TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, hadamard(&qr[i]));
  }

  getQuregAmps(sv, qregistry.registers[qr[0].registry_index], 0, NAMPS);
  TEST_ASSERT_EQUAL_DOUBLE(1.0, creal(sv[0]));
  TEST_ASSERT_EQUAL_DOUBLE(0.0, cimag(sv[0]));
  for (size_t i = 1; i < NAMPS; ++i) {
    sprintf(msg, "Real component: index = %lu", i);
    TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(0.0, creal(sv[i]), msg);

    sprintf(msg, "Imaginary component: index = %lu", i);
    TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(0.0, cimag(sv[i]), msg);
  }
  
  return;
}

void test_cphase(void) {
  return;
}

void test_swap(void) {
  return;
}