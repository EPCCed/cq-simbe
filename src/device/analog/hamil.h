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
cq_status add_ising_interaction_terms(analog_qreg *qreg, cq_hamiltonian *hamiltonian);
cq_status add_xy_interaction_terms(analog_qreg *qreg, cq_hamiltonian *hamiltonian);

cq_status add_driving_terms(analog_qreg *qreg, cq_hamiltonian *hamiltonian);

// static and move to .c?
cq_status add_driving_local_term(ptrdiff_t target, cq_hamiltonian *hamiltonian);
cq_status add_driving_global_term(analog_qreg *qreg, cq_hamiltonian *hamiltonian);

cq_status update_sys_terms(const analog_qreg *qreg, cq_hamiltonian *hamiltonian);
cq_status zero_driving_terms(ptrdiff_t start, ptrdiff_t end,
                            const analog_qreg *qreg,
                            cq_hamiltonian *hamiltonian);

cq_status reset_hamiltonian(cq_hamiltonian *hamiltonian);

typedef enum term_modifier_type {
    TERM_MODIFIER_FREQ_COS = 0,
    TERM_MODIFIER_FREQ_SIN,
    TERM_MODIFIER_DETUNING,
    TERM_MODIFIER_COUNT
} term_modifier_type;

double freq_cos_modifier(double freq, double phase, double detuning);
double freq_sin_modifier(double freq, double phase, double detuning);
double detuning_modifier(double freq, double phase, double detuning);

typedef double (*term_modifier)(double, double, double);
extern term_modifier term_modifiers[TERM_MODIFIER_COUNT];

#endif // CQ_ANALOG_HAMIL_H
