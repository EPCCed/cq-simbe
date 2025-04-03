#include <complex.h>
#include "unity.h"
#include "host_ops.h"
#include "datatypes.h"
#include "src/device/resources.h"
#include "src/device/device_utils.h"
#include "quest/include/qureg.h"
#include "device_ops.h"
#include "test_device_ops.h"

const size_t NQUBITS = 5;
const unsigned int NAMPS = 1 << NQUBITS;
qubit * qr;

void setUp() {
  alloc_qureg(&qr, NQUBITS);  
  return;
}

void tearDown() {
  free_qureg(&qr);
  return;
}

void test_qureg_setters(void) {
  char msg[64];
  unsigned long long state;
  complex double sv[NAMPS];

  for (state = 0; state < NAMPS; ++state) {
    TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, set_qureg(qr, state, NQUBITS));
    getQuregAmps(sv, qregistry.registers[qr[0].registry_index], 0, NAMPS);
    for (size_t i = 0; i < NAMPS; ++i) {
      sprintf(msg, "Real component: state = %llu, index = %lu", state, i);
      if (i == state) {
        TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(1.0, creal(sv[i]), msg);
      } else {
        TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(0.0, creal(sv[i]), msg);
      }

      sprintf(msg, "Imaginary component: state = %llu, index = %lu", 
        state, i);
      TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(0.0, cimag(sv[i]), msg);
    }
  }

  return;
}

void test_qureg_measure(void) {
  char msg[32];
  cstate cr[NQUBITS];
  cstate expected[NQUBITS];
  unsigned long long state;
  
  for (state = 0; state < NAMPS; ++state) {
    sprintf(msg, "state = %llu", state);

    set_qureg(qr, state, NQUBITS);
    qindex_to_cstate(state, expected, NQUBITS);

    TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, measure_qureg(qr, NQUBITS, cr));
    TEST_ASSERT_EQUAL_INT16_ARRAY(expected, cr, NQUBITS);
  }

  return;
}