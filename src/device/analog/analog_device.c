/**
* @author Mateusz Meller
*
* @copyright Copyright (c) 2025
* UK Research and Innovation,
* Science and Technology Facilities Council,
* Hartree Centre
*/

#include "analog_device.h"

#include "analog_qreg.h"
#include "channel.h"
#include "hamil.h"

#include <assert.h>
#include <stdio.h>

static analog_device device = {0};

static cq_status validate_qreg_is_init(analog_qreg *qreg) {
    if (!qreg) return CQ_ERROR;
    if (!qreg->in_use) {
        printf("Error: The qreg with index: %d was not initialised in analog mode.\n", qreg->id);
        return CQ_ERROR;
    }
    return CQ_SUCCESS;
}

static cq_status setup_device_params(device_mode mode) {
    // Pulser settings based on Pasqal device
    // used to reproduce experiments
    device.sample_rate = 0.25;
    device.min_pulse_duration = 16.0;
    device.max_pulse_duration = 60000.0;
    //device.ising_coefficient = 865723.02;
    device.ising_coefficient = 5420158.53;
    device.xy_coefficient = 3700.0;
    device.max_atom_dist_from_origin = 50.0;
    device.min_dist_between_atom = 4.0;
    device.max_num_shots = 2000;
    device.mode = mode;

    return CQ_SUCCESS;
}

cq_status enable_analog_mode(device_mode mode) {
    if (device.is_initialized) {
        printf("Error: Attempting to re-initialize analog mode.\n");
        return CQ_ERROR;
    }
    setup_device_params(mode);

    device.is_initialized = true;
    return CQ_SUCCESS;
}

cq_status enable_analog_qreg(int qreg_id, int num_qubits) {
    assert(qreg_id > -1 && qreg_id < __CQ_ANALOG_MAX_NUM_QUREGS__);
    assert(num_qubits > 0 && num_qubits <= __CQ_ANALOG_MAX_NUM_QUBITS__);

    if (!device.is_initialized) {
        printf("Error: Attempting to run analog operation without "
               "analog mode on. From: %s\n", __func__);
        return CQ_ERROR;
    }

    analog_qreg *qreg = &device.qregs[qreg_id];
    if (qreg->in_use) {
        printf("Error: Attempting to use qreg that is already in use.\n");
        return CQ_ERROR;
    }

    cq_hamiltonian *hamiltonian = &device.hamiltonians[qreg_id];
    init_qreg(qreg, qreg_id, num_qubits, hamiltonian);
    return CQ_SUCCESS;
}

cq_status disable_analog_qreg(int qreg_id) {
    assert(qreg_id > -1 && qreg_id < __CQ_ANALOG_MAX_NUM_QUREGS__);

    if (!device.is_initialized) {
        printf("Error: Attempting to run analog operation without "
               "analog mode on. From: %s\n", __func__);
        return CQ_ERROR;
    }

    analog_qreg *qreg = &device.qregs[qreg_id];
    if (!qreg->in_use) {
        printf("Error: Attempting to disable analog mode on qreg which was already disabled.\n");
        return CQ_ERROR;
    }
    reset_qreg(qreg);

    cq_hamiltonian *hamiltonian = &device.hamiltonians[qreg_id];
    reset_hamiltonian(hamiltonian);

    return CQ_SUCCESS;
}

cq_status get_global_channel(channel *ch, int type, int qreg_id) {
    assert(ch != NULL);
    assert(qreg_id > -1 && qreg_id < __CQ_ANALOG_MAX_NUM_QUREGS__);
    //if (validate_channel_type(type, LOCAL) == CQ_ERROR) return CQ_ERROR;

    if (!device.is_initialized) {
        printf("Error: Attempting to run analog operation without "
               "analog mode on. From: %s\n", __func__);
        return CQ_ERROR;
    }

    analog_qreg *qreg = &device.qregs[qreg_id];
    if (validate_qreg_is_init(qreg) == CQ_ERROR) {
        printf("Error: Attempting to get channel to operate on a uninitialized qreg.\n");
        return CQ_ERROR;
    }

    for (int i = 0; i < qreg->num_channels; ++i) {
        if (qreg->channels[i].type == type) {
            return copy_channel(ch, &qreg->channels[i]);
        }
    }
    printf("Error: Couldn't find the requested channel. Check your inputs.\n");
    return CQ_ERROR;
}

cq_status get_local_channel(channel *ch, int type, int target, int qreg_id) {
    assert(ch != NULL);
    assert(qreg_id > -1 && qreg_id < __CQ_ANALOG_MAX_NUM_QUREGS__);
    assert(target > -1 && qreg_id < __CQ_ANALOG_MAX_NUM_QUBITS__);
    //if (validate_channel_type(type, LOCAL) == CQ_ERROR) return CQ_ERROR;
    if (!device.is_initialized) {
        printf("Error: Attempting to run analog operation without "
               "analog mode on. From: %s\n", __func__);
        return CQ_ERROR;
    }

    analog_qreg *qreg = &device.qregs[qreg_id];
    if (validate_qreg_is_init(qreg) == CQ_ERROR) {
        printf("Error: Attempting to get channel to operate on "
               "a uninitialized qreg. From: %s\n", __func__);
        return CQ_ERROR;
    }

    if (target >= qreg->num_qubits) {
        printf("Error: Provided target (%d) is larger than number of qubits "
               "in given register (qreg id: %d). From: %s\n",
               target, qreg->num_qubits, __func__);
        return CQ_ERROR;
    }

    for (int i = 0; i < qreg->num_channels; ++i) {
        if (qreg->channels[i].type == type &&
            qreg->channels[i].target == target) {
            return copy_channel(ch, &qreg->channels[i]);
        }
    }
    printf("Error: Couldn't find the requested channel. Check your inputs.\n");
    return CQ_ERROR;
}

cq_status is_analog_device_init(void) {
    if (!device.is_initialized) {
        printf("Error: Attempting to run analog operation but "
               "analog device was not initialised!\n");
        return CQ_ERROR;
    }
    return CQ_SUCCESS;
}

cq_hamiltonian * get_hamiltonian(const analog_qreg *qreg) {
    assert(device.is_initialized);
    assert(qreg != NULL);
    return &device.hamiltonians[qreg->id];
}

analog_qreg * get_qreg(int qreg_id) {
    assert(device.is_initialized);
    assert(qreg_id > -1 && qreg_id < __CQ_ANALOG_MAX_NUM_QUREGS__);
    return &device.qregs[qreg_id];
}

double get_device_sample_rate(void) {
    assert(device.is_initialized);
    return device.sample_rate;
}

double get_device_min_pulse_duration(void) {
    assert(device.is_initialized);
    return device.min_pulse_duration;
}

double get_device_max_pulse_duration(void) {
    assert(device.is_initialized);
    return device.max_pulse_duration;
}

double get_device_ising_coeff(void) {
    assert(device.is_initialized);
    return device.ising_coefficient;
}

double get_device_xy_coeff(void) {
    assert(device.is_initialized);
    return device.xy_coefficient;
}

double get_device_min_atom_dist(void) {
    assert(device.is_initialized);
    return device.min_dist_between_atom;
}

int get_device_max_num_shots(void) {
    assert(device.is_initialized);
    return device.max_num_shots;
}

device_mode get_device_operating_mode(void) {
    assert(device.is_initialized);
    return device.mode;
}

ptrdiff_t duration_to_samples(double duration) {
    return (ptrdiff_t)(device.sample_rate * duration);
}

double samples_to_duration(ptrdiff_t num_samples) {
    return (double)num_samples / device.sample_rate;
}

static const char *get_mode_str(device_mode mode) {
    switch (mode) {
        case RYDBERG:
            return "RYDBERG";
        case XY:
            return "XY";
        default:
            return "Unknown";
    }
}

void print_analog_device(void) {
    assert(device.is_initialized);
    printf("==============================================================\n"
        "Analog Device Specification:\n"
           "Operating Modes: RYDBERG, XY\n"
           "Selected mode: %s\n"
           "Sample rate: %f\n"
           "Min pulse duration: %f\n"
           "Max pulse duration: %f\n"
           "Max number of shots: %d\n"
           "Ising coefficient: %f\n"
           "\n-------- If RYDBERG or XY: --------\n"
           "XY coefficient: %f\n"
           "Min distance between atoms: %f\n"
           "Max atom distance from origin: %f\n"
           "==============================================================\n",
            get_mode_str(device.mode),
            device.sample_rate,
            device.min_pulse_duration       ,
            device.max_pulse_duration       ,
            device.max_num_shots            ,
            device.ising_coefficient        ,
            device.xy_coefficient           ,
            device.min_dist_between_atom    ,
            device.max_atom_dist_from_origin
    );
}
