/**
* @author Mateusz Meller
*
* @copyright Copyright (c) 2025
* UK Research and Innovation,
* Science and Technology Facilities Council,
* Hartree Centre
*/

#ifndef CQ_ANALOG_DATATYPES_H
#define CQ_ANALOG_DATATYPES_H

#include "analog.h"

#include <stdbool.h>
#include <stddef.h>

// ---------- structs ---------------------------------------------------------
typedef struct term_range {
    ptrdiff_t start;
    ptrdiff_t end;
} term_range;

typedef enum channel_type {
    RYDBERG_GLOBAL = 0,
    RYDBERG_LOCAL = 1,
    RAMAN_LOCAL = 2,
    DMM_GLOBAL = 3,
    MW_GLOBAL = 4
} channel_type;

typedef enum addressing {
    LOCAL = 0,
    GLOBAL = 1
} addressing;

typedef struct channel_params {
    double max_freq;            // rad/microsec
    double max_detuning;        // rad/microsec
    double min_amp;             // rad/microsec
    double min_retarget_dt;     // ns
    double retarget_delay;      // ns
    double sample_rate;         // GHz
    double min_pulse_duration;  // ns
    int max_targets;            // -1 -> no max i.e. global
    addressing addressing;
    ptrdiff_t qreg_id;               // on which qreg the channel operates
} channel_params;

#define __CQ_ANALOG_MAX_NUM_QUBITS__ 59
// NOTE: PAULI_STR_LEN == MAX_NUM_QUBITS
#define __CQ_ANALOG_MAX_PAULI_STR_LEN__ __CQ_ANALOG_MAX_NUM_QUBITS__
typedef struct ham_term {
    char paulis[__CQ_ANALOG_MAX_PAULI_STR_LEN__];
    int indices[__CQ_ANALOG_MAX_PAULI_STR_LEN__];
    ptrdiff_t num_paulis;

    int var_idx;
    double sign;
} ham_term;

#define __CQ_ANALOG_MAX_NUM_HAM_TERMS__ 8192
typedef struct cq_hamiltonian {
    double real[__CQ_ANALOG_MAX_NUM_HAM_TERMS__];
    double imag[__CQ_ANALOG_MAX_NUM_HAM_TERMS__];
    ham_term terms[__CQ_ANALOG_MAX_NUM_HAM_TERMS__];
    ptrdiff_t num_terms;

} cq_hamiltonian;

typedef struct qpos {
    double x;
    double y;
    double z;
} qpos;

#define __CQ_ANALOG_MAX_NUM_CHANNELS__ __CQ_ANALOG_MAX_NUM_QUBITS__ + 1
typedef struct analog_qreg {
    ptrdiff_t id;
    bool in_use;
    ptrdiff_t num_qubits;
    qpos qubit_pos[__CQ_ANALOG_MAX_NUM_QUBITS__];
    term_range sys_terms_range;

    channel channels[__CQ_ANALOG_MAX_NUM_CHANNELS__];
    ptrdiff_t num_channels;
    term_range channels_ranges[__CQ_ANALOG_MAX_NUM_CHANNELS__];

    channel_params channel_params[4];
} analog_qreg;

#define __CQ_ANALOG_MAX_NUM_QUREGS__ 64
// TODO: Device could have func ptr to coupling function
// this could be provided by the user and later used for calculating
// interaction
typedef double (*coupling_func)(double *q0, double *q1); 
typedef struct analog_device {
    double sample_rate;                 // GHz i.e 1/ns
    double min_pulse_duration;          // ns
    double max_pulse_duration;          // ns
    double interaction_coeff;
    double min_qubit_dist;              // micrometers
    ptrdiff_t max_num_shots;
    bool is_initialized;
    device_mode mode;
    coupling_func coupler;

    analog_qreg qregs[__CQ_ANALOG_MAX_NUM_QUREGS__];
    cq_hamiltonian hamiltonians[__CQ_ANALOG_MAX_NUM_QUREGS__];

} analog_device;

#define __CQ_ANALOG_EPSILON__ 0.0000000001

#endif // CQ_ANALOG_DATATYPES_H

