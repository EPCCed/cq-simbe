#ifndef CQ_TEST_EXECUTORS_H
#define CQ_TEST_EXECUTORS_H

#include <math.h>
#include "datatypes.h"
#include "kernel_utils.h"
#include "device_ops.h"
#include "qasm_gates.h"
#include "tests/test_qkerns.h"

void test_first_run(void);
void test_nmeasure(void);
void test_nshots(void);
void test_bad_inputs(void);
void test_kernel_abort(void);

#endif