#include "unity.h"
#include "datatypes.h"
#include "kernel_utils.h"
#include "src/host-device/comms.h"
#include "src/device/control.h"
#include "src/device/resources.h"
#include "quest/include/environment.h"
#include "tests/test_qkerns.h"
#include "test_device_control.h"

void setUp(void) {
  return;
}

void tearDown(void) {
  return;
}

void test_initialise_simulator(void) {
  const int VERBOSE = 1;

  TEST_ASSERT_FALSE(isQuESTEnvInit());
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, initialise_simulator((void*) &VERBOSE));
  TEST_ASSERT(isQuESTEnvInit());
  TEST_ASSERT_EQUAL_size_t(0, qregistry.num_registers);
  TEST_ASSERT_EQUAL_INT(CQ_WARNING, initialise_simulator((void *) &VERBOSE));

  return;
}

void test_run_qkernel(void) {
  const int NQUBITS = 4;
  cstate cr[NQUBITS];
  qubit qr[NQUBITS];

  device_alloc_params dap = {
    .NQUBITS = NQUBITS,
    .qregistry_idx = 0,
    .status = CQ_ERROR
  };
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, device_alloc_qureg((void*) &dap));
  for (size_t i = 0; i < NQUBITS; ++i) {
    qr[i].N = NQUBITS;
    qr[i].offset = i;
    qr[i].registry_index = dap.qregistry_idx;
  }

  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, register_qkern(zero_init_full_qft));

  cq_exec ex_ur;
  init_exec_handle(NQUBITS, 1, NQUBITS, &ex_ur);
  ex_ur.fname = "unregistered_kernel";
  ex_ur.qreg = qr;
  ex_ur.creg = cr;

  cq_exec ex_zqft;
  init_exec_handle(NQUBITS, 1, NQUBITS, &ex_zqft);
  ex_zqft.fname = "zero_init_full_qft";
  ex_zqft.qreg = qr;
  ex_zqft.creg = cr;

  TEST_ASSERT_EQUAL_INT(CQ_ERROR, run_qkernel((void*) &ex_ur));
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, run_qkernel((void*) &ex_zqft));

  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, device_dealloc_qureg((void*) &dap));

  return;
}

void test_finalise_simulator(void) {
  const int VERBOSE = 1;
  
  TEST_ASSERT(isQuESTEnvInit());
  TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, finalise_simulator((void*) &VERBOSE));
  TEST_ASSERT_FALSE(isQuESTEnvInit());
  TEST_ASSERT_EQUAL_size_t(0, qregistry.num_registers);

  TEST_ASSERT_EQUAL_INT(CQ_WARNING, finalise_simulator((void *) &VERBOSE));

  return;
}