/**
* @author Mateusz Meller
*
* @copyright Copyright (c) 2025
* UK Research and Innovation,
* Science and Technology Facilities Council,
* Hartree Centre
*/

#ifndef CQ_ANALOG_WAVEFORMS_H
#define CQ_ANALOG_WAVEFORMS_H

#include "analog_datatypes.h"

cq_status gaussian_wf(double *samples, double duration, double amp, double sigma);
cq_status gaussian_sqr_wf(double *samples, double duration, double amp,
                          double sigma, double width);

cq_status interpolated_wf(double *samples, double duration,
                          double *points, int num_points);
cq_status sech_wf(double *samples, double duration, double amp, double sigma);
cq_status sin_wf(double *samples, double duration, double amp,
                 double freq, double phase);
cq_status cos_wf(double *samples, double duration, double amp,
                 double freq, double phase);
cq_status blackman_wf(double *samples, double duration, double amp);
cq_status saw_wf(double *samples, double duration, double amp,
                 double freq, double phase);
cq_status custom_wf(double *samples, double *values, int num_samples);
cq_status composite_wf(double *samples, double **waveforms,
                       ptrdiff_t *num_samples, int num_waveforms,
                       ptrdiff_t *total_num_samples);

// utils
cq_status validate_duration(double duration, double min_duration, double max_duration);
cq_status validate_sampling(double sample_rate, double duration, ptrdiff_t num_samples);

#endif // CQ_ANALOG_WAVEFORMS_H
