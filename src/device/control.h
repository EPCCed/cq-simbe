#ifndef DEVICE_CONTROL_H
#define DEVICE_CONTROL_H

extern int (*control_registry[8])(void *);

int initialise_simulator(void *);

int abort_current_kernel(void *);

int finalise_simulator(void *);

int run_qkernel(void *);

int run_pqkernel(void *);

int test_control_fn(void *);

#endif 