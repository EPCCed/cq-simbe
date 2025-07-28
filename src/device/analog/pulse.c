/**
* @author Mateusz Meller
*
* @copyright Copyright (c) 2025
* UK Research and Innovation,
* Science and Technology Facilities Council,
* Hartree Centre
*/

#include "pulse.h"

#include "analog_device.h"
#include "simulator.h"
#include "waveforms.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

static cq_status can_channel_play_pulse(channel *ch, pulse *pulse) {

    assert(ch != NULL);
    assert(ch->params != NULL);
    assert(pulse != NULL);

    channel_params *params = (channel_params *)ch->params;
    if (pulse->duration < params->min_pulse_duration) {
        printf("Error: Pulse duration is shorter than supported by channel.\n");
        return CQ_ERROR;
    }

    if (pulse->duration > get_device_max_pulse_duration()) {
        printf("Error: Pulse duration is longer than supported by device.\n");
        return CQ_ERROR;
    }

    if (pulse->duration < get_device_min_pulse_duration()) {
        printf("Error: Pulse duration is shorter than supported by device.\n");
        return CQ_ERROR;
    }

    assert(params->sample_rate == get_device_sample_rate());
    if (params->sample_rate != get_device_sample_rate()) {
        printf("(internal) Error: Channel sample rate is not supported by device.\n");
        return CQ_ERROR;
    }

    for (ptrdiff_t i = 0; i < pulse->num_samples; ++i) {
        if (pulse->freq[i] > params->max_freq) {
            printf("Error: Pulse frequency is above the channel threshold.\n");
            return CQ_ERROR;
        }
        if (pulse->detuning[i] > params->max_detuning) {
            printf("Error: Pulse detuning is above the channel threshold.\n");
            return CQ_ERROR;
        }

        if (pulse->detuning[i] < -params->max_detuning) {
            printf("Error: Pulse detuning is below the channel threshold.\n");
            return CQ_ERROR;
        }

    }

    return CQ_SUCCESS;
}

cq_status init_pulse(pulse *pulse, double duration) {
    assert(pulse != NULL);
    if (validate_duration(
            duration,
            get_device_min_pulse_duration(),
            get_device_max_pulse_duration()) == CQ_ERROR) return CQ_ERROR;

    pulse->duration = duration;
    double sample_rate = get_device_sample_rate();
    ptrdiff_t num_samples = (ptrdiff_t)(duration * sample_rate);

    validate_sampling(sample_rate, duration, num_samples);

    pulse->num_samples = num_samples;

    pulse->freq = malloc(num_samples * sizeof(double));
    if (!pulse->freq) {
        printf("Error: Failed to allocate freq array in pulse. "
		"From: %s\n", __func__);
	return CQ_ERROR;
    }

    pulse->phase = malloc(num_samples * sizeof(double));
    if (!pulse->phase) {
        printf("Error: Failed to allocate phase array in pulse. "
		"From: %s\n", __func__);
	return CQ_ERROR;
    }
 
    pulse->detuning = malloc(num_samples * sizeof(double));
    if (!pulse->detuning) {
        printf("Error: Failed to allocate detuning array in pulse. "
		"From: %s\n", __func__);
	return CQ_ERROR;
    }
 
    for (ptrdiff_t i = 0; i < num_samples; ++i) {
        pulse->freq[i] = 0.0;
        pulse->phase[i] = 0.0;
        pulse->detuning[i] = 0.0;
    }

    return CQ_SUCCESS;
}

cq_status free_pulse(pulse *pulse) {
    assert(pulse != NULL);
    assert(pulse->freq != NULL);
    assert(pulse->phase != NULL);
    assert(pulse->detuning != NULL);

    free(pulse->freq);
    free(pulse->phase);
    free(pulse->detuning);
    pulse->freq = NULL;
    pulse->phase = NULL;
    pulse->detuning = NULL;
    return CQ_SUCCESS;
}

cq_status play(channel *ch, pulse *pulse) {
    assert(ch != NULL);
    assert(ch->params != NULL);
    assert(pulse != NULL);

    if (can_channel_play_pulse(ch, pulse) == CQ_ERROR) return CQ_ERROR;
    channel_params *params = (channel_params *)ch->params;
    assert(params != NULL);

    int qreg_id = params->qreg_id;
    assert(qreg_id > -1 && qreg_id < __CQ_ANALOG_MAX_NUM_QUREGS__);

    analog_qreg *qreg = get_qreg(qreg_id);
    assert(qreg != NULL);
    assert(qreg->in_use);

    cq_hamiltonian *hamiltonian = get_hamiltonian(qreg);
    assert(hamiltonian != NULL);

    simulate_pulse(ch, pulse, qreg, hamiltonian);
    ch->time += pulse->duration;

    return CQ_SUCCESS;
}

cq_status capture(channel *ch, pulse *pulse, int *result, int shots) {
    assert(ch != NULL);
    assert(ch->params != NULL);
    assert(pulse != NULL);
    assert(result != NULL);

    if (can_channel_play_pulse(ch, pulse) == CQ_ERROR) {
        return CQ_ERROR;
    }

    if (shots < 1 || shots > get_device_max_num_shots()) {
        printf("Error: Invalid number of shots (%d). Should be "
               "in [1, %d] range. From: %s\n",
               shots,
               get_device_max_num_shots(),
               __func__);
        return CQ_ERROR;
    }

    channel_params *params = (channel_params *)ch->params;
    assert(params != NULL);
    int qreg_id = params->qreg_id;
    assert(qreg_id > -1 && qreg_id < __CQ_ANALOG_MAX_NUM_QUREGS__);
    analog_qreg *qreg = get_qreg(qreg_id);
    assert(qreg != NULL);
    assert(qreg->in_use);

    for (ptrdiff_t i = 0; i < shots; ++i) {
        result[i] = simulate_capture(ch, qreg);
        ch->time += pulse->duration;
    }

    return CQ_SUCCESS;
}

cq_status delay(channel *ch, double dt) {
    assert(ch != NULL);
    assert(ch->params != NULL);

    if (validate_duration(
            dt,
            get_device_min_pulse_duration(),
            get_device_max_pulse_duration()) == CQ_ERROR) return CQ_ERROR;
    ch->time += dt;
    return CQ_SUCCESS;
}

static cq_status validate_num_channels(int num_channels) {
    if (num_channels < 1 || num_channels > __CQ_ANALOG_MAX_NUM_CHANNELS__) {
        printf("Error: Provided wrong number of channels. "
               "The num_channels should fall in range [1, %d], given: %d\n",
               __CQ_ANALOG_MAX_NUM_CHANNELS__, num_channels);
        return CQ_ERROR;
    }
    return CQ_SUCCESS;
}

cq_status barrier(channel **ch, int num_channels) {
    assert(ch != NULL);

    if (validate_num_channels(num_channels) == CQ_ERROR) return CQ_ERROR;

    double max_time = 0.0;
    for (ptrdiff_t i = 0; i < num_channels; ++i) {
        assert(ch[i] != NULL);
        //if (!ch[i]) return CQ_ERROR;
        if (ch[i]->time > max_time) max_time = ch[i]->time;
    }

    for (ptrdiff_t i = 0; i < num_channels; ++i) {
        if (ch[i]->time < max_time) delay(ch[i], max_time - ch[i]->time);
    }
    return CQ_SUCCESS;
}


