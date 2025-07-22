/**
* @author Mateusz Meller
*
* @copyright Copyright (c) 2025
* UK Research and Innovation,
* Science and Technology Facilities Council,
* Hartree Centre
*/

#include "analog_qreg.h"

#include "analog_device.h"
#include "channel.h"
#include "hamil.h"
#include "simulator.h"

#include <assert.h>
#include <stdio.h>

cq_status init_qreg(analog_qreg *qreg, int qreg_id, int num_qubits, cq_hamiltonian *hamiltonian) {
    assert(qreg != NULL);
    assert(qreg_id > -1 && qreg_id < __CQ_ANALOG_MAX_NUM_QUREGS__);
    assert(num_qubits > 0 && num_qubits <= __CQ_ANALOG_MAX_NUM_QUBITS__);
    assert(hamiltonian != NULL);

    qreg->in_use = true;
    qreg->id = qreg_id;
    qreg->num_qubits = num_qubits;

    init_qubit_pos(qreg->qubit_pos, num_qubits);
    setup_channel_params(qreg);
    add_interaction_terms(qreg, hamiltonian);
    add_driving_terms(qreg, hamiltonian);

    init_simulator_qreg(qreg_id, num_qubits);

    return CQ_SUCCESS;
}

cq_status init_qubit_pos(qpos *qubit_pos, int num_qubits) {
    assert(qubit_pos != NULL);
    assert(num_qubits > 0 && num_qubits <= __CQ_ANALOG_MAX_NUM_QUBITS__);

    double min_dist = get_device_min_atom_dist() + 0.00001;
    for (int i = 0; i < num_qubits; ++i) {
        qubit_pos[i].x = i * min_dist;
        qubit_pos[i].y = 0.0;
        qubit_pos[i].z = 0.0;
    }
    return CQ_SUCCESS;
}

cq_status reset_qreg(analog_qreg *qreg) {
    assert(qreg != NULL);
    if (!qreg->in_use) {
        printf("Error: Attempting to disable analog mode on qreg which was already disabled.\n");
        return CQ_ERROR;
    }

    qreg->in_use = false;

    qreg->num_qubits = 0;
    qreg->sys_terms_range.start = 0;
    qreg->sys_terms_range.end = 0;
    qreg->num_channels = 0;

    for (ptrdiff_t i = 0; i < __CQ_ANALOG_MAX_NUM_QUBITS__; ++i) {
        qreg->qubit_pos[i].x = 0;
        qreg->qubit_pos[i].y = 0;
        qreg->qubit_pos[i].z = 0;
    }

    for (ptrdiff_t i = 0; i < __CQ_ANALOG_MAX_NUM_CHANNELS__; ++i) {
        qreg->channels_ranges[i].start = 0;
        qreg->channels_ranges[i].end = 0;

        qreg->channels[i].id = 0;
        qreg->channels[i].type = 0;
        qreg->channels[i].target = 0;
        qreg->channels[i].time = 0.0;
        qreg->channels[i].params = NULL;
    }

    reset_simulator_qreg(qreg->id);
    return CQ_SUCCESS;
}

cq_status update_qreg_pos(const qpos *positions, int num_qubits, int qreg_id) {
    assert(positions != NULL);
    assert(num_qubits > 0 && num_qubits < __CQ_ANALOG_MAX_NUM_QUBITS__);
    assert(qreg_id > -1 && qreg_id < __CQ_ANALOG_MAX_NUM_QUREGS__);

    analog_qreg *qreg = get_qreg(qreg_id);
    assert(qreg != NULL);

    if (!qreg->in_use) {
        printf("Error: The qreg with index: %d was not initialised "
               "in analog mode. From: %s\n",
               qreg->id, __func__);
        return CQ_ERROR;
    }

    if (num_qubits > qreg->num_qubits) {
        printf("Error: The passed num_qubits (%d) is larger than the size "
               "of register (%d). From: %s\n",
               num_qubits, qreg->num_qubits, __func__);
        return CQ_ERROR;
    }

    for (int i = 0; i < num_qubits; ++i) {
        qreg->qubit_pos[i].x = positions[i].x;
        qreg->qubit_pos[i].y = positions[i].y;
        qreg->qubit_pos[i].z = positions[i].z;
    }

    if(update_sys_terms(qreg, get_hamiltonian(qreg)) == CQ_ERROR) return CQ_ERROR;
    sync_simulator(get_hamiltonian(qreg), qreg_id);

    return CQ_SUCCESS;
}


cq_status print_qpos(int qreg_id) {
    assert(qreg_id > -1 && qreg_id < __CQ_ANALOG_MAX_NUM_QUREGS__);

    analog_qreg *qreg = get_qreg(qreg_id);
    assert(qreg != NULL);

    if (!qreg->in_use) {
        printf("Error: The qreg with index: %d was not initialised "
               "in analog mode. From: %s\n",
               qreg->id, __func__);
        return CQ_ERROR;
    }

    printf("Qubit positions in qreg %d:\n", qreg_id);
    for (int i = 0; i < qreg->num_qubits; ++i) {
        printf("q%d: %f %f %f\n", i,
            qreg->qubit_pos[i].x,
            qreg->qubit_pos[i].y,
            qreg->qubit_pos[i].z
        );
    }

    return CQ_SUCCESS;
}
