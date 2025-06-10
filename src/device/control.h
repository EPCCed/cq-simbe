#ifndef DEVICE_CONTROL_H
#define DEVICE_CONTROL_H

extern cq_status (*control_registry[8])(void *);

cq_status initialise_simulator(void *);

cq_status abort_current_kernel(void *);

cq_status finalise_simulator(void *);

cq_status run_qkernel(void *);

cq_status run_pqkernel(void *);

cq_status test_control_fn(void *);

#endif 