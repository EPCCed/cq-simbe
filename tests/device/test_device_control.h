#ifndef CQ_TEST_DEVICE_CONTROL_H
#define CQ_TEST_DEVICE_CONTROL_H

// double initialisation and finalisation 
// blocked by user-facing functions
void test_initialise_simulator(void);
void test_run_qkernel(void);
void test_finalise_simulator(void);

#endif