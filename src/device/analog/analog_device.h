/**
* @author Mateusz Meller
*
* @copyright Copyright (c) 2025
* UK Research and Innovation,
* Science and Technology Facilities Council,
* Hartree Centre
*/

#ifndef CQ_ANALOG_DEVICE_H
#define CQ_ANALOG_DEVICE_H

#include "analog_datatypes.h"

cq_status enable_analog_mode(device_mode mode);
cq_status enable_analog_qreg(int qreg_id, int num_qubits);
cq_status disable_analog_qreg(int qreg_id);
cq_status get_global_channel(channel *ch, int type, int qreg_id);
cq_status get_local_channel(channel *ch, int type, int target, int qreg_id);
cq_status is_analog_device_init(void);

cq_hamiltonian * get_hamiltonian(const analog_qreg *qreg);
analog_qreg * get_qreg(int qreg_id);

double get_device_sample_rate(void);
double get_device_min_pulse_duration(void);
double get_device_max_pulse_duration(void);
double get_device_ising_coeff(void);
double get_device_xy_coeff(void);
double get_device_min_qubit_dist(void);
int get_device_max_num_shots(void);
device_mode get_device_operating_mode(void);

ptrdiff_t duration_to_samples(double duration);
double samples_to_duration(ptrdiff_t num_samples);

void print_analog_device(void);

#endif // CQ_ANALOG_DEVICE_H
