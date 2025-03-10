#ifndef DEVICE_CONTROL_H
#define DEVICE_CONTROL_H

extern int (*control_registry[5])(void *);

int initialise_simulator(void *);

int abort_current_kernel(void *);

int finalise_simulator(void *);

#endif 