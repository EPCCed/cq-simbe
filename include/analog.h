/**
* @author Mateusz Meller
*
* @copyright Copyright (c) 2025
* UK Research and Innovation,
* Science and Technology Facilities Council,
* Hartree Centre
*/

#ifndef CQ_ANALOG_H
#define CQ_ANALOG_H

#include "datatypes.h"
#include <stddef.h>

typedef enum device_mode {
    ISING,
    XY
} device_mode;

typedef struct channel {
    int id;
    int type;
    int target;
    double time;
    void* params;
} channel;

#define __CQ_ANALOG_MAX_NUM_SAMPLES__ 4096
typedef struct pulse {
    //double freq[__CQ_ANALOG_MAX_NUM_SAMPLES__];
    //double phase[__CQ_ANALOG_MAX_NUM_SAMPLES__];
    //double detuning[__CQ_ANALOG_MAX_NUM_SAMPLES__];
    double *freq;
    double *phase;
    double *detuning;

    double duration;
    ptrdiff_t num_samples;
} pulse;

cq_status cq_print_analog_device(void);
cq_status cq_print_avail_channels(void);
cq_status cq_print_channel(channel *ch);
cq_status cq_print_qpos(int qreg_id);
ptrdiff_t cq_duration_to_samples(double duration);
double cq_samples_to_duration(ptrdiff_t num_samples);

cq_status cq_enable_analog_mode(int mode);
cq_status cq_enable_analog_qreg(qubit *qr);
cq_status cq_disable_analog_qreg(qubit *qr);

cq_status cq_get_channel(channel *ch, int type, qubit *qr, qubit *target);
cq_status cq_retarget_channel(channel *ch, qubit *new_target);

//cq_status cq_get_global_channel(channel *ch, int type, int qreg_id);
//cq_status cq_get_local_channel(channel *ch, int type, int target, int qreg_id);

//cq_status cq_update_qreg_pos(const qpos *new_positions, int num_qubits, int qreg_id);
cq_status cq_set_qubit_pos(const double *new_positions, qubit *qr);

cq_status cq_init_pulse(pulse *pulse, double duration);
cq_status cq_free_pulse(pulse *pulse);
cq_status cq_play(channel *ch, pulse *pulse);
cq_status cq_capture(channel *ch, pulse *pulse, int *result, int shots);
cq_status cq_delay(channel *ch, double dt);
cq_status cq_barrier(channel **ch, int num_channels);

// =============================== Waveforms ==================================
cq_status cq_gaussian_wf(double *samples, double duration, double amp, double sigma);

// TODO: consider removing as it has limited applicability
cq_status cq_gaussian_sqr_wf(double *samples, double duration, double amp,
                          double sigma, double width);

cq_status cq_interpolated_wf(double *samples, double duration,
                          double *points, int num_points);
cq_status cq_sech_wf(double *samples, double duration, double amp, double sigma);
cq_status cq_sin_wf(double *samples, double duration, double amp,
                 double freq, double phase);
cq_status cq_cos_wf(double *samples, double duration, double amp,
                 double freq, double phase);
cq_status cq_blackman_wf(double *samples, double duration, double amp);
cq_status cq_saw_wf(double *samples, double duration, double amp,
                 double freq, double phase);
cq_status cq_custom_wf(double *samples, double *values, int num_samples);
cq_status cq_composite_wf(double *samples, double **waveforms,
                       ptrdiff_t *num_samples, int num_waveforms,
                       ptrdiff_t *total_num_samples);

#endif
