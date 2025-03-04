#ifndef DEVICE_CONTROL_H
#define DEVICE_CONTROL_H

int initialise_quantum_device(void *);

int abort_current_kernel(void *);

int finalise_quantum_device(void *);

#endif 