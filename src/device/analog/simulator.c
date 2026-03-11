/**
* @author Mateusz Meller
*
* @copyright Copyright (c) 2025
* UK Research and Innovation,
* Science and Technology Facilities Council,
* Hartree Centre
*/

#include "simulator.h"

#include "analog_datatypes.h"
#include "hamil.h"

#include "../resources.h"
#include "quest.h"

#include <assert.h>
#include <stdio.h>
#include <stdbool.h>

static PauliStrSum quest_hamiltonians[__CQ_ANALOG_MAX_NUM_QUREGS__] = {0};

static cq_status get_pauli_strings(cq_hamiltonian *hamiltonian, PauliStr *pauli_strings) {
    assert(hamiltonian != NULL);
    assert(pauli_strings != NULL);
    assert(hamiltonian->num_terms < __CQ_ANALOG_MAX_NUM_HAM_TERMS__);

    for (ptrdiff_t i = 0; i < hamiltonian->num_terms; ++i) {
        const PauliStr quest_pauli_str = getPauliStr(
            hamiltonian->terms[i].paulis,
            hamiltonian->terms[i].indices,
            hamiltonian->terms[i].num_paulis);

        pauli_strings[i] = quest_pauli_str;
    }

    return CQ_SUCCESS;
}

static cq_status init_quest_hamiltonian(cq_hamiltonian *hamiltonian,
                                PauliStrSum *quest_hamiltonian) {
    assert(hamiltonian != NULL);

    PauliStr pauli_strings[__CQ_ANALOG_MAX_NUM_HAM_TERMS__] = {0};
    get_pauli_strings(hamiltonian, pauli_strings);

    qcomp coeffs[__CQ_ANALOG_MAX_NUM_HAM_TERMS__] = {0};
    for (ptrdiff_t i = 0; i < hamiltonian->num_terms; ++i) {
        coeffs[i] = getQcomp(hamiltonian->real[i], hamiltonian->imag[i]);
    }

    *quest_hamiltonian = createPauliStrSum(pauli_strings, coeffs, hamiltonian->num_terms);
    //reportPauliStrSum(*quest_hamiltonian);

    return CQ_SUCCESS;
}

cq_status simulate_pulse(channel *ch, pulse *pulse, analog_qreg *qreg, cq_hamiltonian *hamiltonian) {
    assert(ch != NULL);
    assert(ch->params != NULL);
    assert(pulse != NULL);
    assert(qreg != NULL);
    assert(hamiltonian != NULL);

    ptrdiff_t qreg_id = qreg->id;

    if (quest_hamiltonians[qreg_id].numTerms == 0) {
        init_quest_hamiltonian(hamiltonian, &quest_hamiltonians[qreg_id]);
    }

    channel_params *params = (channel_params *)ch->params;
    double dt = params->sample_rate;
    ptrdiff_t num_samples = pulse->num_samples;
    for (ptrdiff_t i = 0; i < num_samples; ++i) {
        for (ptrdiff_t j = qreg->channels_ranges[ch->id].start;
             j < qreg->channels_ranges[ch->id].end; ++j) {

            // TODO should coeffs be imag?
            //double complex coeff = 0.0;
            double coeff = 0.0;
            double sign = hamiltonian->terms[j].sign;
            coeff = sign * term_modifiers[hamiltonian->terms[j].var_idx](
                            pulse->freq[i],
                            pulse->phase[i],
                            pulse->detuning[i]);

            //hamiltonian.coeffs.data[j] = coeff;
            // TODO: do I need imag after all?
            hamiltonian->real[j] = coeff;
            quest_hamiltonians[qreg_id].coeffs[j] = coeff;
        }

        applyTrotterizedPauliStrSumGadget(qregistry.registers[qreg_id],
                                          quest_hamiltonians[qreg_id],
                                          dt, 2, 4, false);
    }

    ptrdiff_t start = qreg->channels_ranges[ch->id].start;
    ptrdiff_t end = qreg->channels_ranges[ch->id].end;
    zero_driving_terms(start, end, qreg, hamiltonian);

    for (ptrdiff_t i = start; i < end; ++i) {
        quest_hamiltonians[qreg_id].coeffs[i] = 0.0;
    }

    return CQ_SUCCESS;
}

int simulate_capture(channel *ch, analog_qreg *qreg) {
    assert(ch != NULL);
    assert(ch->params != NULL);
    assert(qreg != NULL);

    return applyQubitMeasurement(
            qregistry.registers[qreg->id],
            ch->target);
}

void sync_simulator(cq_hamiltonian *hamiltonian, ptrdiff_t qreg_id) {
    assert(hamiltonian != NULL);
    assert(qreg_id > -1 && qreg_id < __CQ_ANALOG_MAX_NUM_QUREGS__);

    if (quest_hamiltonians[qreg_id].numTerms == 0) {
        init_quest_hamiltonian(hamiltonian, &quest_hamiltonians[qreg_id]);
    }

    for (ptrdiff_t i = 0; i < hamiltonian->num_terms; ++i) {
        quest_hamiltonians[qreg_id].coeffs[i] = getQcomp(hamiltonian->real[i],
                                                         hamiltonian->imag[i]);
    }
}

void print_statevec(ptrdiff_t qreg_id) {
    assert(qreg_id > -1 && qreg_id < __CQ_ANALOG_MAX_NUM_QUREGS__);
    Qureg quest_qureg = qregistry.registers[qreg_id];
    #define printbits_n(x,n) for (int j=n;j;j--,putchar('0'|(x>>j)&1))

    for (ptrdiff_t i = 0; i < quest_qureg.numAmps; ++i) {
        double prob = creal(quest_qureg.cpuAmps[i]) * creal(quest_qureg.cpuAmps[i]) +
                        cimag(quest_qureg.cpuAmps[i]) * cimag(quest_qureg.cpuAmps[i]);

        printbits_n(i, quest_qureg.numQubits);
        printf(" %f\n", prob);
    }

    #undef printbits_n
}

void init_simulator_qreg(ptrdiff_t qreg_id, ptrdiff_t num_qubits) {
    if (!isQuESTEnvInit()) {
        initQuESTEnv();
    }

    if (qregistry.available[qreg_id]) {
        qregistry.available[qreg_id] = true;
        qregistry.registers[qreg_id] = createQureg((int)num_qubits);
    }
}

void reset_simulator_qreg(ptrdiff_t qreg_id) {
    assert(qreg_id > -1 && qreg_id < __CQ_ANALOG_MAX_NUM_QUREGS__);
    initZeroState(qregistry.registers[qreg_id]);
    //qregistry.available[qreg_id] = false;
}
