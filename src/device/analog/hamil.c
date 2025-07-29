/**
* @author Mateusz Meller
*
* @copyright Copyright (c) 2025
* UK Research and Innovation,
* Science and Technology Facilities Council,
* Hartree Centre
*/

#include "hamil.h"

#include "analog_device.h"
#include "utils.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>

term_modifier term_modifiers[TERM_MODIFIER_COUNT] = {
    &freq_cos_modifier,
    &freq_sin_modifier,
    &detuning_modifier
};

static cq_status add_pauli_term(
    cq_hamiltonian *hamiltonian, char pauli,
    int target, int var_idx, double sign) {

    assert(hamiltonian != NULL);
    assert(pauli == 'I' || pauli == 'X' || pauli == 'Y' || pauli == 'Z' );
    assert(target > -1 && target < __CQ_ANALOG_MAX_NUM_QUBITS__);
    assert(var_idx > -1 && var_idx < TERM_MODIFIER_COUNT);
    assert(sign - 1.0 < __CQ_ANALOG_EPSILON__ || sign + 1.0 < __CQ_ANALOG_EPSILON__);

    if (hamiltonian->num_terms >= __CQ_ANALOG_MAX_NUM_HAM_TERMS__) {
        printf("Error: Number of terms in hamiltonian surpasses maximum.\n");
        return CQ_ERROR;
    }

    ptrdiff_t i = hamiltonian->num_terms;
    hamiltonian->terms[i].paulis[0] = pauli;
    hamiltonian->terms[i].indices[0] = target;
    hamiltonian->terms[i].num_paulis = 1;
    hamiltonian->terms[i].var_idx = var_idx;
    hamiltonian->terms[i].sign = sign;
    hamiltonian->real[i] = 0.0;
    hamiltonian->imag[i] = 0.0;
    hamiltonian->num_terms++;

    return CQ_SUCCESS;
}

static double qubit_dist(const qpos *a, const qpos *b) {
    assert(a != NULL);
    assert(b != NULL);

    double dist = 0.0;
    double dx = a->x - b->x;
    double dy = a->y - b->y;
    double dz = a->z - b->z;
    dist = dx * dx + dy * dy + dz * dz;

    return sqrt(dist);
}

static cq_status ising_interaction(const qpos *q0, const qpos *q1, double *result) {
    assert(q0 != NULL);
    assert(q1 != NULL);

    double C = get_device_ising_coeff();
    double dist6 = qubit_dist(q0, q1);
    if (dist6 < get_device_min_qubit_dist()) {
        printf("Error: The distance between atoms is too small. "
               "Minimal distance between atoms is %f, given %f\n"
               "Setting interaction strength to 0.0!",
               get_device_min_qubit_dist(), dist6);
        *result = 0.0;
        return CQ_ERROR;
    }
    dist6 = dist6 * dist6 * dist6 * dist6 * dist6 * dist6;
    *result = (C / dist6) * 0.25;
    return CQ_SUCCESS;
}

static cq_status xy_interaction(const qpos *q0, const qpos *q1, double *result) {
    assert(q0 != NULL);
    assert(q1 != NULL);

    double C3 = get_device_xy_coeff();
    double dist3 = qubit_dist(q0, q1);
    if (dist3 < get_device_min_qubit_dist()) {
        printf("Error: The distance between atoms is too small. "
               "Minimal distance between atoms is %f, given %f\n"
               "Returning interaction strength = 0.0!",
               get_device_min_qubit_dist(), dist3);
        *result = 0.0;
        return CQ_ERROR;
    }
    dist3 = dist3 * dist3 * dist3;
    *result = (C3 / dist3);
    return CQ_SUCCESS;
}

static cq_status interaction(const qpos *q0, const qpos *q1, double *result) {
    assert(q0 != NULL);
    assert(q1 != NULL);

    switch(get_device_operating_mode()) {
        case ISING:
            return ising_interaction(q0, q1, result);
        case XY:
            return xy_interaction(q0, q1, result);
        default:
            return CQ_ERROR;
    }
}

cq_status add_driving_local_term(int target, cq_hamiltonian *hamiltonian) {
    assert(target > -1 && target < __CQ_ANALOG_MAX_NUM_QUBITS__);
    assert(hamiltonian != NULL);

    double sign = 1.0;
    // Adding (w(t)cos(p(t))X + w(t)sin(p(t))Y - d(t)(1 - Z)) / 2
    if (add_pauli_term(hamiltonian,
                        'X', target,
                        TERM_MODIFIER_FREQ_COS,
                        sign) != CQ_SUCCESS) return CQ_ERROR;

    if (add_pauli_term(hamiltonian,
                        'Y', target,
                        TERM_MODIFIER_FREQ_SIN,
                        sign) != CQ_SUCCESS) return CQ_ERROR;

    if (add_pauli_term(hamiltonian,
                        'I', target,
                        TERM_MODIFIER_DETUNING,
                        -sign) != CQ_SUCCESS) return CQ_ERROR;

    if (add_pauli_term(hamiltonian,
                       'Z', target,
                       TERM_MODIFIER_DETUNING,
                       sign) != CQ_SUCCESS) return CQ_ERROR;

    return CQ_SUCCESS;
}

cq_status add_driving_global_term(analog_qreg *qreg, cq_hamiltonian *hamiltonian) {
    assert(qreg != NULL);
    assert(qreg->in_use);
    assert(qreg->num_qubits < __CQ_ANALOG_MAX_NUM_CHANNELS__);
    assert(hamiltonian != NULL);

    channel global_ch = {0};
    global_ch.id = (int)(qreg->num_channels);
    global_ch.type = RYDBERG_GLOBAL;
    global_ch.target = -1;
    global_ch.params = (void *)&qreg->channel_params[RYDBERG_GLOBAL];

    qreg->channels_ranges[global_ch.id].start = hamiltonian->num_terms;
    for (int i = 0; i < qreg->num_qubits; ++i) {
        channel local_ch = {0};
        local_ch.id = (int)(qreg->num_channels + 1);
        local_ch.type = RYDBERG_LOCAL;
        local_ch.target = i;
        local_ch.params = (void *)&qreg->channel_params[RYDBERG_LOCAL];

        qreg->channels_ranges[local_ch.id].start = hamiltonian->num_terms;
        add_driving_local_term(i, hamiltonian);
        qreg->channels_ranges[local_ch.id].end = hamiltonian->num_terms;

        qreg->channels[local_ch.id] = local_ch;
        qreg->num_channels++;
    }
    qreg->channels_ranges[global_ch.id].end = hamiltonian->num_terms;
    qreg->channels[global_ch.id] = global_ch;
    qreg->num_channels++;

    return CQ_SUCCESS;
}

cq_status add_driving_terms(analog_qreg *qreg, cq_hamiltonian *hamiltonian) {
    assert(qreg != NULL);
    assert(qreg->in_use);
    assert(qreg->num_qubits < __CQ_ANALOG_MAX_NUM_CHANNELS__);
    assert(hamiltonian != NULL);

    add_driving_global_term(qreg, hamiltonian);
    return CQ_SUCCESS;
}

static void print_hamil(cq_hamiltonian *hamiltonian) {
    printf("hamiltonian with %ld terms:\n", hamiltonian->num_terms);
    for (ptrdiff_t i = 0; i < hamiltonian->num_terms; ++i) {
        printf("%lf ", hamiltonian->real[i]);
        for (ptrdiff_t j = 0; j < hamiltonian->terms[i].num_paulis; ++j) {
            printf("%c", hamiltonian->terms[i].paulis[j]);
        }
        printf("\n");
    }
}

static cq_status add_two_pauli_term(
    cq_hamiltonian *hamiltonian, char pauli_i, char pauli_j,
    int i, int j, double interaction_str) {

    assert(hamiltonian != NULL);
    assert(hamiltonian->num_terms < __CQ_ANALOG_MAX_NUM_HAM_TERMS__);
    assert(pauli_i == 'I' || pauli_i == 'X' || pauli_i == 'Y' || pauli_i == 'Z' );
    assert(pauli_j == 'I' || pauli_j == 'X' || pauli_j == 'Y' || pauli_j == 'Z' );
    assert(i > -1 && i < __CQ_ANALOG_MAX_NUM_QUBITS__);
    assert(j > -1 && j < __CQ_ANALOG_MAX_NUM_QUBITS__);

    ptrdiff_t term_idx = hamiltonian->num_terms;
    hamiltonian->terms[term_idx].paulis[0] = pauli_i;
    hamiltonian->terms[term_idx].indices[0] = i;
    hamiltonian->terms[term_idx].paulis[1] = pauli_j;
    hamiltonian->terms[term_idx].indices[1] = j;
    hamiltonian->terms[term_idx].num_paulis = 2;
    hamiltonian->real[term_idx] = interaction_str;
    hamiltonian->imag[term_idx] = 0.0;
    hamiltonian->num_terms++;

    if (hamiltonian->num_terms >= __CQ_ANALOG_MAX_NUM_HAM_TERMS__) {
        printf("Error: Too many terms in the hamiltonian.\n");
        return CQ_ERROR;
    }

    return CQ_SUCCESS;
}

cq_status add_xy_interaction_terms(analog_qreg *qreg, cq_hamiltonian *hamiltonian) {
    assert(qreg != NULL);
    assert(qreg->in_use);
    assert(hamiltonian != NULL);

    // assume systerm is C3 / hR**3 sum(j<i)(|1><0|i|0><1|j + |0><1|i|1><0|j)
    // i.e. pauli+i pauli-j + pauli-i pauli+j
    //
    // pauli+ = X + iY
    // pauli- = X - iY
    //
    // so hamil is: (Xi + iYi)(Xj - iYj) + (Xi - iYi)(Xj + iYj)
    // XiXj - iXiYj + iYiXj + YiYj + XiXj + iXiYj - iYiXj + YiYj
    // 2(XiXj + YiYj)

    qreg->sys_terms_range.start = hamiltonian->num_terms;
    for (int i = 0; i < qreg->num_qubits; ++i) {
        for (int j = i + 1; j < qreg->num_qubits; ++j) {
            double interaction_str = 0.0;
	    HANDLE_CQ_ERROR(xy_interaction(&qreg->qubit_pos[i],
                                 &qreg->qubit_pos[j],
                                 &interaction_str));

            interaction_str *= 2.0;
            add_two_pauli_term(hamiltonian, 'X', 'X', i, j, interaction_str);
            add_two_pauli_term(hamiltonian, 'Y', 'Y', i, j, interaction_str);
        }
    }

    return CQ_SUCCESS;
}

cq_status add_interaction_terms(analog_qreg *qreg, cq_hamiltonian *hamiltonian) {
    assert(qreg != NULL);
    assert(qreg->in_use);
    assert(hamiltonian != NULL);

    device_mode mode = get_device_operating_mode();

    switch (mode) {
        case ISING:
            add_ising_interaction_terms(qreg, hamiltonian);
            break;
        case XY:
            add_xy_interaction_terms(qreg, hamiltonian);
            break;
        default:
            printf("UNKNOWN device mode.\n");
            return CQ_ERROR;
    }

    return CQ_SUCCESS;
}



cq_status add_ising_interaction_terms(analog_qreg *qreg, cq_hamiltonian *hamiltonian) {
    assert(qreg != NULL);
    assert(qreg->in_use);
    assert(hamiltonian != NULL);

    // assume systerm is N0N1 where N = (1 - Z) / 2
    // so we get 11 - 1Z - Z1 + ZZ

    qreg->sys_terms_range.start = hamiltonian->num_terms;
    for (int i = 0; i < qreg->num_qubits; ++i) {
        for (int j = i + 1; j < qreg->num_qubits; ++j) {
            double interaction_str = 0.0;
            HANDLE_CQ_ERROR(ising_interaction(&qreg->qubit_pos[i],
                                 &qreg->qubit_pos[j],
                                 &interaction_str));

            add_two_pauli_term(hamiltonian, 'I', 'I', i, j, interaction_str);
            add_two_pauli_term(hamiltonian, 'I', 'Z', i, j, -interaction_str);
            add_two_pauli_term(hamiltonian, 'Z', 'I', i, j, -interaction_str);
            add_two_pauli_term(hamiltonian, 'Z', 'Z', i, j, interaction_str);
        }
    }
    qreg->sys_terms_range.end = hamiltonian->num_terms;
    //print_hamil(hamiltonian);

    return CQ_SUCCESS;
}

cq_status update_sys_terms(const analog_qreg *qreg, cq_hamiltonian *hamiltonian) {
    assert(qreg != NULL);
    assert(qreg->in_use);
    assert(hamiltonian != NULL);

    if (qreg->sys_terms_range.start == qreg->sys_terms_range.end) return CQ_SUCCESS;
    ptrdiff_t coeff_idx = qreg->sys_terms_range.start;
    for (ptrdiff_t i = 0; i < qreg->num_qubits; ++i) {
        for (ptrdiff_t j = i + 1; j < qreg->num_qubits; ++j) {
            double interaction_str = 0.0;
	    HANDLE_CQ_ERROR(interaction(&qreg->qubit_pos[i],
                           &qreg->qubit_pos[j],
                           &interaction_str));

            hamiltonian->real[coeff_idx] = interaction_str;
            hamiltonian->real[coeff_idx + 1] = -interaction_str;
            hamiltonian->real[coeff_idx + 2] = -interaction_str;
            hamiltonian->real[coeff_idx + 3] = interaction_str;
            coeff_idx += 4;
        }
    }
    return CQ_SUCCESS;
}

cq_status zero_driving_terms(
    ptrdiff_t start, ptrdiff_t end,
    const analog_qreg *qreg, cq_hamiltonian *hamiltonian) {
    assert(qreg != NULL);
    assert(qreg->in_use);
    assert(hamiltonian != NULL);
    assert(start > -1 && start < hamiltonian->num_terms);
    assert(end >= start && end <= hamiltonian->num_terms);

    for (ptrdiff_t i = start; i < end; ++i) {
            hamiltonian->real[i] = 0.0;
            hamiltonian->imag[i] = 0.0;
    }

    return CQ_SUCCESS;
}

cq_status reset_hamiltonian(cq_hamiltonian *hamiltonian) {
    assert(hamiltonian != NULL);

    for (ptrdiff_t i = 0; i < hamiltonian->num_terms; ++i) {
        hamiltonian->real[i] = 0.0;
        hamiltonian->imag[i] = 0.0;
        hamiltonian->terms[i].num_paulis = 0;
        hamiltonian->terms[i].var_idx = TERM_MODIFIER_FREQ_COS;
        hamiltonian->terms[i].sign = 1.0;
    }
    hamiltonian->num_terms = 0;
    return CQ_SUCCESS;
}

#define FREQ_SCALING_FACTOR 0.5
#define DETUNING_SCALING_FACTOR 0.5

double freq_cos_modifier(double freq, double phase, double detuning) {
    return FREQ_SCALING_FACTOR * freq * cos(phase);
}

double freq_sin_modifier(double freq, double phase, double detuning) {

    return FREQ_SCALING_FACTOR * freq * sin(phase);
}

double detuning_modifier(double freq, double phase, double detuning) {
    return DETUNING_SCALING_FACTOR * detuning;
}

#undef DETUNING_SCALING_FACTOR
#undef FREQ_SCALING_FACTOR
