#ifndef CQ_TEST_EXECUTORS_H
#define CQ_TEST_EXECUTORS_H

#include <math.h>
#include "datatypes.h"
#include "kernels.h"
#include "device_ops.h"
#include "qasm_gates.h"
#include "test_qkerns.h"

void test_first_run(void);
void test_nmeasure(void);
void test_nshots(void);
void test_bad_inputs(void);

#endif