#include <complex.h>
#include "unity.h"
#include "host_ops.h"
#include "datatypes.h"
#include "utils.h"
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

  for (size_t tgt = 0; tgt < NQUBITS; ++tgt) {
    TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, set_qubit(qr[tgt], 0));
  }
  getQuregAmps(sv, qregistry.registers[qr[0].registry_index], 0, NAMPS);
  TEST_ASSERT_EQUAL_DOUBLE(1.0, creal(sv[0]));
  TEST_ASSERT_EQUAL_DOUBLE(0.0, cimag(sv[0]));
  TEST_ASSERT_EACH_EQUAL_DOUBLE(0.0, sv+1, 2 * (NAMPS-1));

  size_t hot_amp = 0;
  for (size_t tgt = 0; tgt < NQUBITS; ++tgt) {
    TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, set_qubit(qr[tgt], 1));

    hot_amp += (1lu << tgt);
    getQuregAmps(sv, qregistry.registers[qr[0].registry_index], 0, NAMPS);
    for (size_t i = 0; i < NAMPS; ++i) {
      sprintf(msg, "target = %lu, i = %lu", tgt, i);
      if (i == hot_amp) {
        TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(1.0, creal(sv[i]), msg);
      } else {
        TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(0.0, cimag(sv[i]), msg);
      }
      TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(0.0, cimag(sv[i]), msg);
    }
  }

  TEST_ASSERT_EQUAL_INT(CQ_ERROR, set_qubit(qr[0], 2));
  TEST_ASSERT_EQUAL_INT(CQ_ERROR, set_qubit(qr[0], -1));

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

  TEST_ASSERT_EQUAL_INT(CQ_ERROR, set_qureg(NULL, 0, NQUBITS));
  TEST_ASSERT_EQUAL_INT(CQ_ERROR, set_qureg(qr, 0, NQUBITS+1));
  TEST_ASSERT_EQUAL_INT(CQ_ERROR, set_qureg(qr, 2, 1));
  TEST_ASSERT_EQUAL_INT(CQ_ERROR, set_qureg(qr, 1lu << NQUBITS, NQUBITS));

  cstate cr[NQUBITS];
  for (size_t i = 0; i < NQUBITS; ++i) {
    cr[i] = 0;
  }

  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, set_qureg_cstate(qr, cr, NQUBITS));

  getQuregAmps(sv, qregistry.registers[qr[0].registry_index], 0, NAMPS);
  TEST_ASSERT_EQUAL_DOUBLE(1.0, creal(sv[0]));
  TEST_ASSERT_EQUAL_DOUBLE(0.0, cimag(sv[0]));
  TEST_ASSERT_EACH_EQUAL_DOUBLE(0.0, sv+1, 2 * (NAMPS-1));

  // set last qubit to 1
  cr[NQUBITS-1] = 1;
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, set_qureg_cstate(qr, cr, NQUBITS));

  getQuregAmps(sv, qregistry.registers[qr[0].registry_index], 0, NAMPS);
  TEST_ASSERT_EACH_EQUAL_DOUBLE(0.0, sv, NAMPS); // 2 * (NAMPS/2)
  TEST_ASSERT_EQUAL_DOUBLE(1.0, creal(sv[NAMPS/2]));
  TEST_ASSERT_EQUAL_DOUBLE(0.0, cimag(sv[NAMPS/2]));
  TEST_ASSERT_EACH_EQUAL_DOUBLE(0.0, sv + (NAMPS/2) + 1, NAMPS - 2); // 2 * (NAMPS/2 -1)

  for (size_t i = 0; i < NQUBITS; ++i) {
    cr[i] = 1;
  }

  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, set_qureg_cstate(qr, cr, NQUBITS));

  getQuregAmps(sv, qregistry.registers[qr[0].registry_index], 0, NAMPS);
  TEST_ASSERT_EACH_EQUAL_DOUBLE(0.0, sv, 2 * (NAMPS-1));
  TEST_ASSERT_EQUAL_DOUBLE(1.0, creal(sv[NAMPS-1]));
  TEST_ASSERT_EQUAL_DOUBLE(0.0, cimag(sv[NAMPS-1]));

  TEST_ASSERT_EQUAL_INT(CQ_ERROR, set_qureg_cstate(NULL, NULL, 0));
  TEST_ASSERT_EQUAL_INT(CQ_ERROR, set_qureg_cstate(qr, NULL, NQUBITS));
  TEST_ASSERT_EQUAL_INT(CQ_ERROR, set_qureg_cstate(NULL, cr, NQUBITS));
  TEST_ASSERT_EQUAL_INT(CQ_ERROR, set_qureg_cstate(qr, cr, NQUBITS+1));
  
  cr[NQUBITS-1] = -1;
  TEST_ASSERT_EQUAL_INT(CQ_ERROR, set_qureg_cstate(qr, cr, NQUBITS));

  cr[NQUBITS-1] = 2;
  TEST_ASSERT_EQUAL_INT(CQ_ERROR, set_qureg_cstate(qr, cr, NQUBITS));

  return;
}

void test_qureg_measure(void) {
  char msg[32];
  cstate cr[NQUBITS];
  cstate expected[NQUBITS];
  unsigned long long state;

  init_creg(NQUBITS, -1, cr);
  set_qureg(qr, 0, NQUBITS);
  for (size_t i = 0; i < NQUBITS; ++i) {
    TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, measure_qubit(&qr[i], &cr[i]));
  }
  TEST_ASSERT_EACH_EQUAL_INT16(0, cr, NQUBITS);

  init_creg(NQUBITS, -1, cr);
  set_qureg(qr, (1lu << NQUBITS) - 1, NQUBITS);
  for (size_t i = 0; i < NQUBITS; ++i) {
    TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, measure_qubit(&qr[i], &cr[i]));
  }
  TEST_ASSERT_EACH_EQUAL_INT16(1, cr, NQUBITS);

  TEST_ASSERT_EQUAL_INT(CQ_ERROR, measure_qubit(NULL, NULL));
  TEST_ASSERT_EQUAL_INT(CQ_ERROR, measure_qubit(&qr[0], NULL));
  TEST_ASSERT_EQUAL_INT(CQ_ERROR, measure_qubit(NULL, &cr[0]));

  for (state = 0; state < NAMPS; ++state) {
    sprintf(msg, "state = %llu", state);

    init_creg(NQUBITS, -1, cr);
    set_qureg(qr, state, NQUBITS);
    qindex_to_cstate(state, expected, NQUBITS);

    TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, measure_qureg(qr, NQUBITS, cr));
    TEST_ASSERT_EQUAL_INT16_ARRAY(expected, cr, NQUBITS);
  }

  TEST_ASSERT_EQUAL_INT(CQ_ERROR, measure_qureg(NULL, NQUBITS, cr));
  TEST_ASSERT_EQUAL_INT(CQ_ERROR, measure_qureg(qr, NQUBITS, NULL));
  TEST_ASSERT_EQUAL_INT(CQ_ERROR, measure_qureg(qr, NQUBITS + 1, cr));

  return;
}