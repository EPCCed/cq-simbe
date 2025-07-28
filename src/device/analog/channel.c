/**
* @author Mateusz Meller
*
* @copyright Copyright (c) 2025
* UK Research and Innovation,
* Science and Technology Facilities Council,
* Hartree Centre
*/

#include "channel.h"
#include <assert.h>
#include <stdio.h>

static cq_status setup_rydberg_local_params(channel_params *params, int qreg_id) {
    assert(params != NULL);

    params->max_freq = 62.83;
    params->max_detuning = 125.7;
    params->min_amp = 0.0;
    params->min_retarget_dt = 220.0;
    params->retarget_delay = 0.0;
    params->sample_rate = 0.25;
    params->min_pulse_duration = 16;
    params->max_targets = 1;
    params->addressing = LOCAL;
    params->qreg_id = qreg_id;

    return CQ_SUCCESS;
}

static cq_status setup_rydberg_global_params(channel_params *params, int qreg_id) {
    assert(params != NULL);

    params->max_freq = 15.71;
    params->max_detuning = 125.7;
    params->min_amp = 0.0;
    params->min_retarget_dt = 0.0;
    params->retarget_delay = 0.0;
    params->sample_rate = 0.25;
    params->min_pulse_duration = 16;
    params->max_targets = -1;
    params->addressing = GLOBAL;
    params->qreg_id = qreg_id;

    return CQ_SUCCESS;
}

static cq_status setup_raman_local_params(channel_params *params, int qreg_id) {
    assert(params != NULL);

    params->max_freq = 62.83;
    params->max_detuning = 125.7;
    params->min_amp = 0.0;
    params->min_retarget_dt = 220.0;
    params->retarget_delay = 0.0;
    params->sample_rate = 0.25;
    params->min_pulse_duration = 16;
    params->max_targets = 1;
    params->addressing = LOCAL;
    params->qreg_id = qreg_id;

    return CQ_SUCCESS;
}

static cq_status setup_dmm_global_params(channel_params *params, int qreg_id) {
    assert(params != NULL);

    params->max_freq = 0.0;
    params->max_detuning = 125.7;
    params->min_amp = 0.0;
    params->min_retarget_dt = 0.0;
    params->retarget_delay = 0.0;
    params->sample_rate = 0.25;
    params->min_pulse_duration = 16;
    params->max_targets = -1;
    params->addressing = GLOBAL;
    params->qreg_id = qreg_id;

    return CQ_SUCCESS;
}

cq_status setup_channel_params(analog_qreg *qreg) {
    assert(qreg != NULL);

    int qreg_id = qreg->id;
    setup_rydberg_global_params(&qreg->channel_params[RYDBERG_GLOBAL], qreg_id);
    setup_rydberg_local_params(&qreg->channel_params[RYDBERG_LOCAL], qreg_id);
    setup_raman_local_params(&qreg->channel_params[RAMAN_LOCAL], qreg_id);
    setup_dmm_global_params(&qreg->channel_params[DMM_GLOBAL], qreg_id);
    return CQ_SUCCESS;
}

cq_status copy_channel(channel *dest, const channel *src) {
    assert(dest != NULL);
    assert(src != NULL);
    assert(src->params != NULL);
    assert(src->id > -1);
    assert(src->target > -2);
    assert(src->target < __CQ_ANALOG_MAX_NUM_QUBITS__);
    assert(src->time > -__CQ_ANALOG_EPSILON__);

    dest->id = src->id;
    dest->type = src->type;
    dest->target = src->target;
    dest->params = src->params;
    dest->time = src->time;

    return CQ_SUCCESS;
}

cq_status retarget_channel(channel *ch, int new_target) {
    assert(ch != NULL);
    assert(new_target > -2);
    assert(new_target < __CQ_ANALOG_MAX_NUM_QUBITS__);
    assert(ch->params != NULL);
 
    // TODO: enforce that only one qubit can be targetted
    ch->target = new_target;
    ch->time += ((channel_params *)ch->params)->retarget_delay;
    return CQ_SUCCESS;
}

void print_avail_channels(void) {
    printf("==============================================================\n"
           "Available Channels:\n"
           "----- GLOBAL:     \n"
            "\tmax_freq = 15.71       \n"
            "\tmax_detuning = 125.7   \n"
            "\tmin_amp = 0.0          \n"
            "\tmin_retarget_dt = 0.0  \n"
            "\tretarget_delay = 0.0   \n"
            "\tsample_rate = 0.25     \n"
            "\tmin_pulse_duration = 16\n"
            "\tmax_targets = -1       \n"
            "\taddressing = GLOBAL    \n\n"

           "----- LOCAL:      \n"
            "\tmax_freq = 62.83       \n"
            "\tmax_detuning = 125.7   \n"
            "\tmin_amp = 0.0          \n"
            "\tmin_retarget_dt = 220.0  \n"
            "\tretarget_delay = 0.0   \n"
            "\tsample_rate = 0.25     \n"
            "\tmin_pulse_duration = 16\n"
            "\tmax_targets = 1       \n"
            "\taddressing = LOCAL    \n"
            "==============================================================\n"
    );
}

static const char *get_channel_type_str(channel_type type) {
    switch (type) {
        case RYDBERG_LOCAL:
            return "LOCAL";
        case RYDBERG_GLOBAL:
            return "GLOBAL";
        default:
            return "Unknown";
    }
}

static const char *get_addressing_str(addressing addressing) {
    switch (addressing) {
        case LOCAL:
            return "LOCAL";
        case GLOBAL:
            return "GLOBAL";
        default:
            return "Unknown";
    }
}

void print_channel(channel *ch) {
    assert(ch != NULL);
    channel_params *params = (channel_params *)ch->params;
    assert(params != NULL);
    printf(
       "----- channel:      \n"
            "\tid: %d\n"
            "\ttype: %s\n"
            "\ttarget: %d\n"
            "\ttime: %f\n"
            "\tmax_freq = %f       \n"
            "\tmax_detuning = %f   \n"
            "\tmin_amp = %f          \n"
            "\tmin_retarget_dt = %f  \n"
            "\tretarget_delay = %f   \n"
            "\tsample_rate = %f    \n"
            "\tmin_pulse_duration = %f\n"
            "\tmax_targets = %d       \n"
            "\taddressing = %s    \n",
            ch->id,
            get_channel_type_str(ch->type),
            ch->target,
            ch->time,
            params->max_freq,
            params->max_detuning,
            params->min_amp,
            params->min_retarget_dt,
            params->retarget_delay,
            params->sample_rate,
            params->min_pulse_duration,
            params->max_targets,
            get_addressing_str(params->addressing)
    );
}


