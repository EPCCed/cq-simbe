/**
* @author Mateusz Meller
*
* @copyright Copyright (c) 2025
* UK Research and Innovation,
* Science and Technology Facilities Council,
* Hartree Centre
*/

#ifndef CQ_ANALOG_HAMIL_H
#define CQ_ANALOG_HAMIL_H

#include "analog_datatypes.h"

cq_status add_interaction_terms(analog_qreg *qreg, cq_hamiltonian *hamiltonian);

// static and move to .c?
cq_status add_rydberg_interaction_terms(analog_qreg *qreg, cq_hamiltonian *hamiltonian);
cq_status add_xy_interaction_terms(analog_qreg *qreg, cq_hamiltonian *hamiltonian);

cq_status add_driving_terms(analog_qreg *qreg, cq_hamiltonian *hamiltonian);

// static and move to .c?
cq_status add_rydberg_local_term(int target, cq_hamiltonian *hamiltonian);
cq_status add_rydberg_global_term(analog_qreg *qreg, cq_hamiltonian *hamiltonian);
cq_status add_raman_local_term(int target, cq_hamiltonian *hamiltonian);

// this applies global detuning zero amp and negative detuing
// adds -h/2 sum e * d(t) Z where e are the weights in the detuing map of atoms
// A DetuningMap associates a set of locations with a set of weights. The locations are the trap coordinates to address and the weights have to be between 0 and 1.
// SO this is Register thing
cq_status add_dmm_global_term(analog_qreg *qreg);

// TODO allow to select SC -- something like dwave with corresponding hamils

cq_status update_sys_terms(const analog_qreg *qreg, cq_hamiltonian *hamiltonian);
cq_status zero_driving_terms(ptrdiff_t start, ptrdiff_t end,
                            const analog_qreg *qreg,
                            cq_hamiltonian *hamiltonian);

cq_status reset_hamiltonian(cq_hamiltonian *hamiltonian);

typedef enum term_modifier_type {
    TERM_MODIFIER_RABI_COS = 0,
    TERM_MODIFIER_RABI_SIN,
    TERM_MODIFIER_DETUNING,
    TERM_MODIFIER_COUNT
} term_modifier_type;

double rabi_freq_cos_modifier(double freq, double phase, double detuning);
double rabi_freq_sin_modifier(double freq, double phase, double detuning);
double detuning_modifier(double freq, double phase, double detuning);

typedef double (*term_modifier)(double, double, double);
extern term_modifier term_modifiers[TERM_MODIFIER_COUNT];

#endif // CQ_ANALOG_HAMIL_H
