#include "unity.h"
#include "datatypes.h"
#include "quest/include/types.h"
#include "test_device_utils.h"
#include "src/device/device_utils.h"

void setUp(void) {
  return;
}

void tearDown(void) {
  return;
}

void test_qindex_to_cstate(void) {
  // 
  cstate onebit = -1;
  cstate twobit[2];
  cstate threebit[3];
  cstate fourbit[4];
  cstate fivebit[5];
  cstate byte[8];
  cstate twobyte[16];
  qindex state;

  state = 0; // 0b0
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, qindex_to_cstate(state, &onebit, 1));
  TEST_ASSERT_EQUAL_INT16(0, onebit);
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, qindex_to_cstate(state, twobit, 2));
  TEST_ASSERT_EACH_EQUAL_INT16(0, twobit, 2);
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, qindex_to_cstate(state, threebit, 3));
  TEST_ASSERT_EACH_EQUAL_INT16(0, threebit, 3);
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, qindex_to_cstate(state, fourbit, 4));
  TEST_ASSERT_EACH_EQUAL_INT16(0, fourbit, 4);
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, qindex_to_cstate(state, fivebit, 5));
  TEST_ASSERT_EACH_EQUAL_INT16(0, fivebit, 5);
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, qindex_to_cstate(state, byte, 8));
  TEST_ASSERT_EACH_EQUAL_INT16(0, byte, 8);
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, qindex_to_cstate(state, twobyte, 16));
  TEST_ASSERT_EACH_EQUAL_INT16(0, twobyte, 16);
  
  state = 1; // 0b01
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, qindex_to_cstate(state, &onebit, 1));
  TEST_ASSERT_EQUAL_INT16(1, onebit);
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, qindex_to_cstate(state, twobit, 2));
  TEST_ASSERT_EQUAL_INT16(0, twobit[0]);
  TEST_ASSERT_EQUAL_INT16(1, twobit[1]);
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, qindex_to_cstate(state, threebit, 3));
  TEST_ASSERT_EACH_EQUAL_INT16(0, threebit, 2);
  TEST_ASSERT_EQUAL_INT16(1, threebit[2]);
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, qindex_to_cstate(state, fourbit, 4));
  TEST_ASSERT_EACH_EQUAL_INT16(0, fourbit, 3);
  TEST_ASSERT_EQUAL_INT16(1, fourbit[3]);
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, qindex_to_cstate(state, fivebit, 5));
  TEST_ASSERT_EACH_EQUAL_INT16(0, fivebit, 4);
  TEST_ASSERT_EQUAL_INT16(1, fivebit[4]);
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, qindex_to_cstate(state, byte, 8));
  TEST_ASSERT_EACH_EQUAL_INT16(0, byte, 7);
  TEST_ASSERT_EQUAL_INT16(1, byte[7]);
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, qindex_to_cstate(state, twobyte, 16));
  TEST_ASSERT_EACH_EQUAL_INT16(0, twobyte, 15);
  TEST_ASSERT_EQUAL_INT16(1, twobyte[15]);

  state = 2; // 0b10
  TEST_ASSERT_EQUAL_INT(CQ_ERROR, qindex_to_cstate(state, &onebit, 1));
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, qindex_to_cstate(state, twobit, 2));
  TEST_ASSERT_EQUAL_INT16(1, twobit[0]);
  TEST_ASSERT_EQUAL_INT16(0, twobit[1]);
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, qindex_to_cstate(state, threebit, 3));
  TEST_ASSERT_EQUAL_INT16(0, threebit[0]);
  TEST_ASSERT_EQUAL_INT16(1, threebit[1]);
  TEST_ASSERT_EQUAL_INT16(0, threebit[2]);
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, qindex_to_cstate(state, fourbit, 4));
  TEST_ASSERT_EACH_EQUAL_INT16(0, fourbit, 2);
  TEST_ASSERT_EQUAL_INT16(1, fourbit[2]);
  TEST_ASSERT_EQUAL_INT16(0, fourbit[3]);
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, qindex_to_cstate(state, fivebit, 5));
  TEST_ASSERT_EACH_EQUAL_INT16(0, fivebit, 3);
  TEST_ASSERT_EQUAL_INT16(1, fivebit[3]);
  TEST_ASSERT_EQUAL_INT16(0, fivebit[4]);
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, qindex_to_cstate(state, byte, 8));
  TEST_ASSERT_EACH_EQUAL_INT16(0, byte, 6);
  TEST_ASSERT_EQUAL_INT16(1, byte[6]);
  TEST_ASSERT_EQUAL_INT16(0, byte[7]);
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, qindex_to_cstate(state, twobyte, 16));
  TEST_ASSERT_EACH_EQUAL_INT16(0, twobyte, 14);
  TEST_ASSERT_EQUAL_INT16(1, twobyte[14]);
  TEST_ASSERT_EQUAL_INT16(0, twobyte[15]);

  state = 3; // 0b11
  TEST_ASSERT_EQUAL_INT(CQ_ERROR, qindex_to_cstate(state, &onebit, 1));
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, qindex_to_cstate(state, twobit, 2));
  TEST_ASSERT_EACH_EQUAL_INT16(1, twobit, 2);
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, qindex_to_cstate(state, threebit, 3));
  TEST_ASSERT_EQUAL_INT16(0, threebit[0]);
  TEST_ASSERT_EACH_EQUAL_INT16(1, &threebit[1], 2);
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, qindex_to_cstate(state, fourbit, 4));
  TEST_ASSERT_EACH_EQUAL_INT16(0, fourbit, 2);
  TEST_ASSERT_EACH_EQUAL_INT16(1, &fourbit[2], 2);
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, qindex_to_cstate(state, fivebit, 5));
  TEST_ASSERT_EACH_EQUAL_INT16(0, fivebit, 3);
  TEST_ASSERT_EACH_EQUAL_INT16(1, &fivebit[3], 2);
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, qindex_to_cstate(state, byte, 8));
  TEST_ASSERT_EACH_EQUAL_INT16(0, byte, 6);
  TEST_ASSERT_EACH_EQUAL_INT16(1, &byte[6], 2);
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, qindex_to_cstate(state, twobyte, 16));
  TEST_ASSERT_EACH_EQUAL_INT16(0, twobyte, 14);
  TEST_ASSERT_EACH_EQUAL_INT16(1, &twobyte[14], 2);

  state = 4; // 0b100
  TEST_ASSERT_EQUAL_INT(CQ_ERROR, qindex_to_cstate(state, &onebit, 1));
  TEST_ASSERT_EQUAL_INT(CQ_ERROR, qindex_to_cstate(state, twobit, 2));
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, qindex_to_cstate(state, threebit, 3));
  TEST_ASSERT_EQUAL_INT16(1, threebit[0]);
  TEST_ASSERT_EACH_EQUAL_INT16(0, &threebit[1], 2);
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, qindex_to_cstate(state, fourbit, 4));
  TEST_ASSERT_EQUAL_INT16(0, fourbit[0]);
  TEST_ASSERT_EQUAL_INT16(1, fourbit[1]);
  TEST_ASSERT_EACH_EQUAL_INT16(0, &fourbit[2], 2);
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, qindex_to_cstate(state, fivebit, 5));
  TEST_ASSERT_EACH_EQUAL_INT16(0, fivebit, 2);
  TEST_ASSERT_EQUAL_INT16(1, fivebit[2]);
  TEST_ASSERT_EACH_EQUAL_INT16(0, &fivebit[3], 2);
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, qindex_to_cstate(state, byte, 8));
  TEST_ASSERT_EACH_EQUAL_INT16(0, byte, 5);
  TEST_ASSERT_EQUAL_INT16(1, byte[5]);
  TEST_ASSERT_EACH_EQUAL_INT16(0, &byte[6], 2);
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, qindex_to_cstate(state, twobyte, 16));
  TEST_ASSERT_EACH_EQUAL_INT16(0, twobyte, 13);
  TEST_ASSERT_EQUAL_INT16(1, twobyte[13]);
  TEST_ASSERT_EACH_EQUAL_INT16(0, &twobyte[14], 2);

  state = 8; // 0b1000
  TEST_ASSERT_EQUAL_INT(CQ_ERROR, qindex_to_cstate(state, &onebit, 1));
  TEST_ASSERT_EQUAL_INT(CQ_ERROR, qindex_to_cstate(state, twobit, 2));
  TEST_ASSERT_EQUAL_INT(CQ_ERROR, qindex_to_cstate(state, threebit, 3));
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, qindex_to_cstate(state, fourbit, 4));
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, qindex_to_cstate(state, fivebit, 5));
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, qindex_to_cstate(state, byte, 8));
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, qindex_to_cstate(state, twobyte, 16));

  // buffer large enough, but BIT_WIDTH too small
  TEST_ASSERT_EQUAL_INT(CQ_ERROR, qindex_to_cstate(state, byte, 3));

  return;
}