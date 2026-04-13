/**
* @author Mateusz Meller
*
* @copyright Copyright (c) 2025
* UK Research and Innovation,
* Science and Technology Facilities Council,
* Hartree Centre
*/

#ifndef CQ_ANALOG_SIMULATOR_H
#define CQ_ANALOG_SIMULATOR_H

#include "analog_datatypes.h"

cq_status simulate_pulse(channel *ch, pulse *pulse, analog_qreg *qreg, cq_hamiltonian *hamiltonian);
int simulate_capture(channel *ch, analog_qreg *qreg);
void sync_simulator(cq_hamiltonian *hamiltonian, ptrdiff_t qreg_id);
void init_simulator_qreg(ptrdiff_t qreg_id, ptrdiff_t num_qubits);
void reset_simulator_qreg(ptrdiff_t qreg_id);

#ifndef NDEBUG
void print_statevec(ptrdiff_t qreg_id);
#endif

#endif // CQ_ANALOG_SIMULATOR_H
