/**
* @author Mateusz Meller
*
* @copyright Copyright (c) 2025
* UK Research and Innovation,
* Science and Technology Facilities Council,
* Hartree Centre
*/

#include "analog.h"

#include "analog_datatypes.h"
#include "analog_device.h"
#include "analog_qreg.h"
#include "channel.h"
#include "pulse.h"
#include "waveforms.h"

#include <stdio.h>

//============================ VALIDATION =====================================
#define HANDLE_CQA_ERR(x)                           \
        {                                           \
            if (x == CQ_ERROR)                      \
            {                                       \
                printf(" From: %s\n", __func__);    \
                return CQ_ERROR;                    \
            }                                       \
        };

static cq_status validate_num_qubits(int num_qubits) {
    if (num_qubits <= 0 || num_qubits > __CQ_ANALOG_MAX_NUM_QUBITS__) {
        printf("Error: Specified number of qubits out-of-bounds. "
               "Should be in [1, %d] range, given %d.",
               __CQ_ANALOG_MAX_NUM_QUBITS__,
               num_qubits);
        return CQ_ERROR;
    }
    return CQ_SUCCESS;
}

static cq_status validate_qubits_idx(int qidx) {
    if (qidx < 0 || qidx >= __CQ_ANALOG_MAX_NUM_QUBITS__) {
        printf("Error: Specified qubit idx out-of-bounds. "
               "Should be in [0, %d] range, given %d.",
               __CQ_ANALOG_MAX_NUM_QUBITS__ - 1,
               qidx);
        return CQ_ERROR;
    }
    return CQ_SUCCESS;
}

static cq_status validate_qreg_id(int qreg_id) {
    if (qreg_id < 0 ||
        qreg_id >= __CQ_ANALOG_MAX_NUM_QUREGS__) {
        printf("Error: Attempting to access qreg with "
               "out-of-bounds index: %d. "
               "Should be in [0, %d] range.",
               qreg_id,
               __CQ_ANALOG_MAX_NUM_QUREGS__ - 1);
        return CQ_ERROR;
    }
    return CQ_SUCCESS;
}

static cq_status validate_channel_type(int type, addressing target) {
    if (type > 4) {
        printf("Error: Provided channel type is undefined "
               "(should be in [0, 4] range, given %d).", type);
        return CQ_ERROR;
    }
    if (target == GLOBAL) {
        if (type == RYDBERG_LOCAL || type == RAMAN_LOCAL) {
            printf("Error: Expected channel addressing is GLOBAL but LOCAL was given.");
            return CQ_ERROR;
        }
    } else if(target == LOCAL) {
        if (type == RYDBERG_GLOBAL || type == DMM_GLOBAL) {
            printf("Error: Expected channel addressing is LOCAL but GLOBAL was given.");
            return CQ_ERROR;
        }
    }
    return CQ_SUCCESS;
}

static cq_status validate_channel(const channel *ch) {
    if (!ch) {
        printf("Error: Channel is nullptr.");
        return CQ_ERROR;
    }
    return CQ_SUCCESS;
}

static cq_status validate_channel_params(const channel *ch) {
    if (!ch->params) {
        printf("Error: Channel params is nullptr.");
        return CQ_ERROR;
    }
    return CQ_SUCCESS;
}

static cq_status validate_channels(channel **ch, int num_channels) {
    if (!ch) {
        printf("Error: Array of channels is null.");
        return CQ_ERROR;
    }

    if (num_channels < 1) {
        printf("Error: Number of passed channels is < 1.");
        return CQ_ERROR;
    }

    for (ptrdiff_t i = 0; i < num_channels; ++i) {
        if (!ch[i] || !ch[i]->params) {
            printf("Error: One of the passed channels is nullptr.");
            return CQ_ERROR;
        }
    }

    return CQ_SUCCESS;
}

static cq_status validate_pulse(const pulse *pulse) {
    if (!pulse) {
        printf("Error: Pulse is nullptr.");
        return CQ_ERROR;
    }
    return CQ_SUCCESS;
}

static cq_status validate_samples(double *samples) {
    if (!samples) {
        printf("Error: Samples are nullptr.");
        return CQ_ERROR;
    }
    return CQ_SUCCESS;
}

static cq_status validate_num_samples(int num_samples) {
    if (num_samples < 1) {
        printf("Error: Specified num_samples < 1.");
        return CQ_ERROR;
    }
    return CQ_SUCCESS;
}

static cq_status validate_values(double *values) {
    if (!values) {
        printf("Error: Values for custom waveform are nullptr.");
        return CQ_ERROR;
    }
    return CQ_SUCCESS;
}

static cq_status validate_result(int *result) {
    if (!result) {
        printf("Error: Result is nullptr.");
        return CQ_ERROR;
    }
    return CQ_SUCCESS;
}

static cq_status validate_num_shots(int shots) {
    if (shots < 1) {
        printf("Error: Number of shots is < 1.");
        return CQ_ERROR;
    }
    return CQ_SUCCESS;
}

static cq_status validate_composite_wf(double *samples, double **waveforms,
                       ptrdiff_t *num_samples, int num_waveforms,
                       ptrdiff_t *total_num_samples) {
    if (!samples) {
        printf("Error: Samples for composite waveform are nullptr.");
        return CQ_ERROR;
    }
    if (!waveforms) {
        printf("Error: Waveforms for composite waveform are nullptr.");
        return CQ_ERROR;
    }
    if (!num_samples) {
        printf("Error: Array of num_samples for composite waveform are nullptr.");
        return CQ_ERROR;
    }
    for (int i = 0; i < num_waveforms; ++i) {
        if (num_samples[i] < 0) {
            printf("Error: num_samples[%d] is negative.", i);
            return CQ_ERROR;
        }
    }
    if (num_waveforms < 1) {
        printf("Error: Passed number of waveforms < 1.");
        return CQ_ERROR;
    }
    if (!total_num_samples) {
        printf("Error: Out-variable total_num_samples is nullptr.");
        return CQ_ERROR;
    }

    return CQ_SUCCESS;
}

static cq_status prevalidate_duration(double duration) {
    if (duration < __CQ_ANALOG_EPSILON__) {
        printf("Error: Duration should be > 0.0.");
        return CQ_ERROR;
    }
    return CQ_SUCCESS;
}

static cq_status validate_freq(double freq) {
    if (freq < 0.0) {
        printf("Error: Frequency should be > 0.0.");
        return CQ_ERROR;
    }
    return CQ_SUCCESS;
}
//========================== ANALOG DEVICE OPS ================================
cq_status cq_enable_analog_mode(device_mode mode) {
    return enable_analog_mode(mode);
}

cq_status cq_enable_analog_qreg(int qreg_id, int num_qubits) {
    HANDLE_CQA_ERR(is_analog_device_init());
    HANDLE_CQA_ERR(validate_qreg_id(qreg_id));
    HANDLE_CQA_ERR(validate_num_qubits(num_qubits));
    return enable_analog_qreg(qreg_id, num_qubits);
}

cq_status cq_disable_analog_qreg(int qreg_id) {
    HANDLE_CQA_ERR(is_analog_device_init());
    HANDLE_CQA_ERR(validate_qreg_id(qreg_id));
    return disable_analog_qreg(qreg_id);
}

cq_status cq_get_global_channel(channel *ch, int type, int qreg_id) {
    HANDLE_CQA_ERR(is_analog_device_init());
    HANDLE_CQA_ERR(validate_channel(ch));
    HANDLE_CQA_ERR(validate_channel_type(type, GLOBAL));
    HANDLE_CQA_ERR(validate_qreg_id(qreg_id));
    return get_global_channel(ch, type, qreg_id);
}

cq_status cq_get_local_channel(channel *ch, int type, int target, int qreg_id) {
    HANDLE_CQA_ERR(is_analog_device_init());
    HANDLE_CQA_ERR(validate_channel(ch));
    HANDLE_CQA_ERR(validate_channel_type(type, LOCAL));
    HANDLE_CQA_ERR(validate_qubits_idx(target));
    HANDLE_CQA_ERR(validate_qreg_id(qreg_id));
    return get_local_channel(ch, type, target, qreg_id);
}

cq_status cq_update_qreg_pos(const qpos *new_positions, int num_qubits, int qreg_id) {
    HANDLE_CQA_ERR(is_analog_device_init());
    if (!new_positions) {
        printf("Error: Passed nullptr positions. From: %s\n", __func__);
        return CQ_ERROR;
    }

    HANDLE_CQA_ERR(validate_num_qubits(num_qubits));
    HANDLE_CQA_ERR(validate_qreg_id(qreg_id));
    return update_qreg_pos(new_positions, num_qubits, qreg_id);
}

//================================ PULSE ======================================
cq_status cq_init_pulse(pulse *pulse, double duration) {
    HANDLE_CQA_ERR(is_analog_device_init());
    HANDLE_CQA_ERR(validate_pulse(pulse));
    return init_pulse(pulse, duration);
}

cq_status cq_play(channel *ch, pulse *pulse) {
    HANDLE_CQA_ERR(is_analog_device_init());
    HANDLE_CQA_ERR(validate_channel(ch));
    HANDLE_CQA_ERR(validate_channel_params(ch));
    HANDLE_CQA_ERR(validate_pulse(pulse));
    return play(ch, pulse);
}

cq_status cq_capture(channel *ch, pulse *pulse, int *result, int shots) {
    HANDLE_CQA_ERR(is_analog_device_init());
    HANDLE_CQA_ERR(validate_channel(ch));
    // can capture only with channels that have valid target
    HANDLE_CQA_ERR(validate_channel_type(ch->type, LOCAL));
    HANDLE_CQA_ERR(validate_channel_params(ch));
    HANDLE_CQA_ERR(validate_pulse(pulse));
    HANDLE_CQA_ERR(validate_result(result));
    HANDLE_CQA_ERR(validate_num_shots(shots));
    return capture(ch, pulse, result, shots);
}

cq_status cq_delay(channel *ch, double dt) {
    HANDLE_CQA_ERR(is_analog_device_init());
    HANDLE_CQA_ERR(validate_channel(ch));
    HANDLE_CQA_ERR(validate_channel_params(ch));
    return delay(ch, dt);
}

cq_status cq_barrier(channel **ch, int num_channels) {
    HANDLE_CQA_ERR(is_analog_device_init());
    HANDLE_CQA_ERR(validate_channels(ch, num_channels));
    return barrier(ch, num_channels);
}

//=============================== WAVEFORMS ===================================

cq_status cq_gaussian_wf(double *samples, double duration, double amp, double sigma) {
    HANDLE_CQA_ERR(is_analog_device_init());
    HANDLE_CQA_ERR(prevalidate_duration(duration));
    HANDLE_CQA_ERR(validate_samples(samples));
    return gaussian_wf(samples, duration, amp, sigma);
}

cq_status cq_gaussian_sqr_wf(double *samples, double duration, double amp,
                          double sigma, double width) {
    printf("Error: Not implemented!\n");
    return CQ_ERROR;

    HANDLE_CQA_ERR(validate_samples(samples));
    if (width > duration) {
        printf("Error: Specified width would produce negative risefall. "
               "Width must be < duration. From: %s\n", __func__);
        return CQ_ERROR;
    }

    return gaussian_sqr_wf(samples, duration, amp,
                          sigma, width);
}

cq_status cq_interpolated_wf(double *samples, double duration,
                          double *points, int num_points) {
    HANDLE_CQA_ERR(is_analog_device_init());
    HANDLE_CQA_ERR(prevalidate_duration(duration));
    HANDLE_CQA_ERR(validate_samples(samples));
    if (!points) {
        printf("Error: Passed nullptr instead of array of points. "
               "From: %s\n", __func__);
        return CQ_ERROR;
    }

    if (num_points < 2) {
        printf("Error: Specified number of points is < 2. "
               "Must pass at least 2 points. From: %s\n", __func__);
        return CQ_ERROR;
    }

    return interpolated_wf(samples, duration,
                          points, num_points);
}

cq_status cq_sech_wf(double *samples, double duration, double amp, double sigma) {
    HANDLE_CQA_ERR(is_analog_device_init());
    HANDLE_CQA_ERR(prevalidate_duration(duration));
    HANDLE_CQA_ERR(validate_samples(samples));
    return sech_wf(samples, duration, amp, sigma);
}

cq_status cq_sin_wf(double *samples, double duration, double amp,
                 double freq, double phase) {
    HANDLE_CQA_ERR(is_analog_device_init());
    HANDLE_CQA_ERR(prevalidate_duration(duration));
    HANDLE_CQA_ERR(validate_samples(samples));
    HANDLE_CQA_ERR(validate_freq(freq));
    return sin_wf(samples, duration, amp,
                 freq, phase);
}

cq_status cq_cos_wf(double *samples, double duration, double amp,
                 double freq, double phase) {
    HANDLE_CQA_ERR(is_analog_device_init());
    HANDLE_CQA_ERR(prevalidate_duration(duration));
    HANDLE_CQA_ERR(validate_samples(samples));
    HANDLE_CQA_ERR(validate_freq(freq));
    return cos_wf(samples, duration, amp,
                 freq, phase);
}

cq_status cq_blackman_wf(double *samples, double duration, double amp) {
    HANDLE_CQA_ERR(is_analog_device_init());
    HANDLE_CQA_ERR(prevalidate_duration(duration));
    HANDLE_CQA_ERR(validate_samples(samples));
    return blackman_wf(samples, duration, amp);
}

cq_status cq_saw_wf(double *samples, double duration, double amp,
                 double freq, double phase) {
    HANDLE_CQA_ERR(is_analog_device_init());
    HANDLE_CQA_ERR(prevalidate_duration(duration));
    HANDLE_CQA_ERR(validate_samples(samples));
    HANDLE_CQA_ERR(validate_freq(freq));
    return saw_wf(samples, duration, amp,
                 freq, phase);
}

cq_status cq_custom_wf(double *samples, double *values, int num_samples) {
    HANDLE_CQA_ERR(is_analog_device_init());
    HANDLE_CQA_ERR(validate_samples(samples));
    HANDLE_CQA_ERR(validate_values(values));
    HANDLE_CQA_ERR(validate_num_samples(num_samples));
    return custom_wf(samples, values, num_samples);
}

cq_status cq_composite_wf(double *samples, double **waveforms,
                       ptrdiff_t *num_samples, int num_waveforms,
                       ptrdiff_t *total_num_samples) {
    HANDLE_CQA_ERR(is_analog_device_init());
    HANDLE_CQA_ERR(validate_composite_wf(samples, waveforms, num_samples,
                                         num_waveforms, total_num_samples));
    return composite_wf(samples, waveforms,
                       num_samples, num_waveforms,
                       total_num_samples);
}

cq_status cq_print_analog_device(void) {
    HANDLE_CQA_ERR(is_analog_device_init());
    print_analog_device();
    return CQ_SUCCESS;
}
cq_status cq_print_avail_channels(void) {
    HANDLE_CQA_ERR(is_analog_device_init());
    print_avail_channels();
    return CQ_SUCCESS;
}

cq_status cq_print_channel(channel *ch) {
    HANDLE_CQA_ERR(is_analog_device_init());
    HANDLE_CQA_ERR(validate_channel(ch));
    HANDLE_CQA_ERR(validate_channel_params(ch));
    print_channel(ch);
    return CQ_SUCCESS;
}

cq_status cq_print_qpos(int qreg_id) {
    HANDLE_CQA_ERR(is_analog_device_init());
    HANDLE_CQA_ERR(validate_qreg_id(qreg_id));
    HANDLE_CQA_ERR(print_qpos(qreg_id));
    return CQ_SUCCESS;
}

ptrdiff_t cq_duration_to_samples(double duration) {
    HANDLE_CQA_ERR(is_analog_device_init());
    if (duration < 0.0) return 0;
    return duration_to_samples(duration);
}

double cq_samples_to_duration(ptrdiff_t num_samples) {
    HANDLE_CQA_ERR(is_analog_device_init());
    if (num_samples < 0) return 0.0;
    return samples_to_duration(num_samples);
}

#undef HANDLE_CQA_ERR
