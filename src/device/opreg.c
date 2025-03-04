#include "opreg.h"
#include "control.h"

int (*control_registry[3])(void *) =  {
  initialise_quantum_device,
  abort_current_kernel,
  finalise_quantum_device
};