/**
* @author Mateusz Meller
*
* @copyright Copyright (c) 2025
* UK Research and Innovation,
* Science and Technology Facilities Council,
* Hartree Centre
*/

#include "waveforms.h"

#include "analog_device.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>

cq_status gaussian_wf(double *samples, double duration,
                      double amp, double sigma) {

    assert(samples != NULL);
    if (validate_duration(duration,
                          get_device_min_pulse_duration(),
                          get_device_max_pulse_duration()) == CQ_ERROR) return CQ_ERROR;

    double sample_rate = get_device_sample_rate();
    ptrdiff_t num_samples = (ptrdiff_t)(duration * sample_rate);

    if (validate_sampling(sample_rate, duration, num_samples) == CQ_ERROR) return CQ_ERROR;
    ptrdiff_t center = (ptrdiff_t)round(((double)num_samples / 2.0));
    for (ptrdiff_t i = 0; i < num_samples; ++i) {
        samples[i] = amp * exp(-((double)((i - center) * (i - center)) / (2 * sigma * sigma)));
    }

    return CQ_SUCCESS;
}

static double lifted_gaussian_sqr(double x, double risefall,
                                  double width, double sigma) {
    double sigma_sqr = sigma * sigma;
    if (x < risefall) {
        double numerator = x - risefall;
        double numerator_sqr = numerator * numerator;
        return exp(-0.5 * numerator_sqr / sigma_sqr);
    } else if (x < risefall + width) {
        return 1.0;
    } else {
        double offset = risefall + width;
        double numerator = x - offset;
        double numerator_sqr = numerator * numerator;
        return exp(-0.5 * numerator_sqr / sigma_sqr);
    }

}

cq_status gaussian_sqr_wf(double *samples, double duration, double amp,
                          double sigma, double width) {
    assert(samples != NULL);
    if (validate_duration(duration,
                          get_device_min_pulse_duration(),
                          get_device_max_pulse_duration()) == CQ_ERROR) return CQ_ERROR;

    double sample_rate = get_device_sample_rate();
    ptrdiff_t num_samples = (ptrdiff_t)(duration * sample_rate);
    if (validate_sampling(sample_rate, duration, num_samples) == CQ_ERROR) return CQ_ERROR;
    double risefall = (duration - width) / 2.0;

    double df_minus_one = lifted_gaussian_sqr(-1.0, risefall, width, sigma);
    for (ptrdiff_t i = 0; i < num_samples; ++i) {
        double df = lifted_gaussian_sqr(((double)(i)),
                                        risefall, width, sigma);

        samples[i] = amp * (df - df_minus_one) / (1 - df_minus_one);
    }

    return CQ_SUCCESS;
}

// cubic polynomial specified by a, b, c, d
// num equations 4N - 4
#define __CQ_ANALOG_INTERPOLATION_MAX_POINTS__ 8
#define __CQ_ANALOG_MAX_NUM_EQUATIONS__ 4 * __CQ_ANALOG_INTERPOLATION_MAX_POINTS__ - 4
static double abs_mat_el(double value) {
    if (value < 0.0) return -value;
    return value;
}

static ptrdiff_t find_pivot(
    double mat[__CQ_ANALOG_MAX_NUM_EQUATIONS__][__CQ_ANALOG_MAX_NUM_EQUATIONS__],
    ptrdiff_t nrows, ptrdiff_t start, ptrdiff_t col) {

    assert(mat != NULL);
    assert(nrows > 0 && nrows <= __CQ_ANALOG_MAX_NUM_EQUATIONS__);
    assert(start >= 0 && start < __CQ_ANALOG_MAX_NUM_EQUATIONS__);
    assert(col >= 0 && col < __CQ_ANALOG_MAX_NUM_EQUATIONS__);

    double max = mat[start][col];
    ptrdiff_t argmax = start;
    for (ptrdiff_t i = start + 1; i < nrows; ++i) {
        if (abs_mat_el(mat[i][col]) > abs_mat_el(max)) {
            max = mat[i][col];
            argmax = i;
        }
    }
    return argmax;
}

static void swap_rows(
    double mat[__CQ_ANALOG_MAX_NUM_EQUATIONS__][__CQ_ANALOG_MAX_NUM_EQUATIONS__],
    ptrdiff_t ncols, ptrdiff_t i, ptrdiff_t j) {

    assert(mat != NULL);
    assert(ncols > 0 && ncols <= __CQ_ANALOG_MAX_NUM_EQUATIONS__);
    assert(i >= 0 && i < __CQ_ANALOG_MAX_NUM_EQUATIONS__);
    assert(j >= 0 && j < __CQ_ANALOG_MAX_NUM_EQUATIONS__);

    if (i != j) {
        for (ptrdiff_t k = 0; k < ncols; ++k) {
            double tmp = mat[i][k];
            mat[i][k] = mat[j][k];
            mat[j][k] = tmp;
        }
    }
}

static void gaussian_elim(
    double mat[__CQ_ANALOG_MAX_NUM_EQUATIONS__][__CQ_ANALOG_MAX_NUM_EQUATIONS__],
    ptrdiff_t nrows, ptrdiff_t ncols) {

    assert(mat != NULL);
    assert(nrows > 0 && nrows <= __CQ_ANALOG_MAX_NUM_EQUATIONS__);
    assert(ncols > 0 && ncols <= __CQ_ANALOG_MAX_NUM_EQUATIONS__);

    ptrdiff_t h = 0; /* Initialization of the pivot row */
    ptrdiff_t k = 0; /* Initialization of the pivot column */

    while (h < nrows && k < ncols) {
        /* Find the k-th pivot: */
        ptrdiff_t pivot_row = find_pivot(mat, nrows, h, k);
        if (mat[pivot_row][k] == 0) {
            ++k;
        } else {
            swap_rows(mat, ncols, h, pivot_row);
            for (ptrdiff_t i = h + 1; i < nrows; ++i) {
                double f = mat[i][k] / mat[h][k];
                mat[i][k] = 0.0;
                for (ptrdiff_t j = k + 1; j < ncols; ++j) {
                    mat[i][j] -= mat[h][j] * f;
                }
            }
        }
        ++h;
        ++k;
    }
}

static void build_spline_problem(
    double *points, int num_points, double interval,
    double mat[__CQ_ANALOG_MAX_NUM_EQUATIONS__][__CQ_ANALOG_MAX_NUM_EQUATIONS__],
    int *nrows_out, int *ncols_out) {

    assert(points != NULL);
    assert(num_points > 1);
    assert(interval > 0.0);
    assert(mat != NULL);
    assert(nrows_out != NULL);
    assert(ncols_out != NULL);

    const int mat_size = 4 * num_points - 4;
    const int nrows = mat_size;
    const int ncols = nrows + 1;
    const int num_poly = num_points - 1;

    *nrows_out = nrows;
    *ncols_out = ncols;

    int row = 0;

    // Cubic polynomials
    for (ptrdiff_t i = 0; i < num_poly; i++, row += 2) {
        // Poly i passes through points i and i+1.
        double x = (double)i * interval;
        mat[row][4 * i] =   x * x * x;
        mat[row][4 * i + 1] = x * x;
        mat[row][4 * i + 2] = x;
        mat[row][4 * i + 3] = 1.0;
        mat[row][ncols - 1] = points[i];

        double x_next = x + interval;
        mat[row + 1][4 * i] = x_next * x_next * x_next;
        mat[row + 1][4 * i + 1] = x_next * x_next;
        mat[row + 1][4 * i + 2] = x_next;
        mat[row + 1][4 * i + 3] = 1.0;
        mat[row + 1][ncols - 1] = points[i + 1];
    }

    // 1st derivative matching
    for (ptrdiff_t i = 0; i < num_poly - 1; i++) {
        // Poly i and poly i+1 must have the same first derivative at
        // point i+1.
        double x_next = (double)(i + 1) * interval;
        mat[row][4 * i] = 3.0 * x_next * x_next;
        mat[row][4 * i + 1] = 2.0 * x_next;
        mat[row][4 * i + 2] = 1.0;
        mat[row][4 * (i + 1)] = -3.0 * x_next * x_next;
        mat[row][4 * (i + 1) + 1] = -2.0 * x_next;
        mat[row][4 * (i + 1) + 2] = -1.0;
        ++row;
    }

    // 2nd derivative matching
    for (ptrdiff_t i = 0; i < num_poly - 1; i++) {
        // Poly i and poly i+1 must have the same second derivative at
        // point i+1.
        double x_next = (double)(i + 1) * interval;
        mat[row][4 * i] = 6.0 * x_next;
        mat[row][4 * i + 1] = 2.0;
        mat[row][4 * (i + 1)] = -6.0 * x_next;
        mat[row][4 * (i + 1) + 1] = -2.0;
        ++row;
    }

    // Boundry conditions
    mat[row][0] = 0.0;
    mat[row][1] = 2.0;
    ++row;
    mat[row][4 * (num_poly - 1)] = 6.0 * num_poly * interval;
    mat[row][4 * (num_poly - 1) + 1] = 2.0;
}

static cq_status reduce_mat(
    double *vec,
    double mat[__CQ_ANALOG_MAX_NUM_EQUATIONS__][__CQ_ANALOG_MAX_NUM_EQUATIONS__],
    int nrows, int ncols, int num_points) {

    assert(vec != NULL);
    assert(mat != NULL);
    assert(nrows > 0 && nrows <= __CQ_ANALOG_MAX_NUM_EQUATIONS__);
    assert(ncols > 0 && ncols <= __CQ_ANALOG_MAX_NUM_EQUATIONS__);
    assert(num_points > 1);

    const int mat_size = 4 * num_points - 4;
    for (ptrdiff_t i = mat_size - 1; i >= 0; --i) {
        // For each row, take its pivot and divide the last column by it,
        // then eliminate the pivot from all rows above.
        double pivot = mat[i][i];
        double abs_pivot = pivot;
        if (pivot < 0.0) abs_pivot = -abs_pivot;
        if (abs_pivot < 0.0000000001) {
            printf("Error: Waveform cannot be interpolated by cubic splines. "
                   "From: %s", __func__);
            return CQ_ERROR;
        }

        for (ptrdiff_t j = i - 1; j >= 0; --j) {
            double f = mat[j][i] / pivot;
            mat[j][i] = 0.0;
            mat[j][ncols - 1] -= mat[i][ncols - 1] * f;
        }
        mat[i][i] = 1.0;
        mat[i][ncols - 1] /= pivot;
        vec[i] = mat[i][ncols - 1];
    }
    return CQ_SUCCESS;
}

static void interpolate(double *samples, ptrdiff_t num_samples,
                        double *vec, int num_points, double interval) {
    assert(samples != NULL);
    assert(num_samples > 0);
    assert(vec != NULL);
    assert(num_points > 1);
    assert(interval > 0.0);

    int num_poly = num_points - 1;
    double dx = (double)(num_samples) / (double)(num_samples - 1);
    for (ptrdiff_t i = 0; i < num_poly; ++i) {
        ptrdiff_t start = i * (ptrdiff_t)interval;
        ptrdiff_t end = start + (ptrdiff_t)interval;

        for (ptrdiff_t j = start; j < end; ++j) {
            double x = (double)j * dx;

            ptrdiff_t row = i * 4;
            double y =  vec[row] * x * x * x +
                        vec[row + 1] * x * x +
                        vec[row + 2] * x +
                        vec[row + 3];
            samples[j] = y;
        }
    }
}

cq_status interpolated_wf(double *samples, double duration,
                              double *points, int num_points) {
    assert(samples != NULL);
    assert(points != NULL);
    assert(num_points > 1);
    if (num_points > __CQ_ANALOG_INTERPOLATION_MAX_POINTS__) {
        printf("Error: Interpolated waveform currently supports up "
               "to %d points. From: %s\n",
               __CQ_ANALOG_INTERPOLATION_MAX_POINTS__,
               __func__);
        return CQ_ERROR;
    }

    if (validate_duration(duration,
            get_device_min_pulse_duration(),
            get_device_max_pulse_duration()) == CQ_ERROR) return CQ_ERROR;


    double sample_rate = get_device_sample_rate();
    ptrdiff_t num_samples = (ptrdiff_t)(duration * sample_rate);
    if (validate_sampling(sample_rate, duration, num_samples) == CQ_ERROR) return CQ_ERROR;
    double interval = (double)num_samples / (double)(num_points - 1);

    double mat[__CQ_ANALOG_MAX_NUM_EQUATIONS__][__CQ_ANALOG_MAX_NUM_EQUATIONS__] = {0};
    double vec[__CQ_ANALOG_MAX_NUM_EQUATIONS__] = {0};

    int nrows;
    int ncols;
    build_spline_problem(points, num_points, interval, mat, &nrows, &ncols);
    gaussian_elim(mat, nrows, ncols);
    if(reduce_mat(vec, mat, nrows, ncols, num_points) == CQ_ERROR) {
        return CQ_ERROR;
    }
    interpolate(samples, num_samples, vec, num_points, interval);

    return CQ_SUCCESS;
}

#undef __CQ_ANALOG_MAX_NUM_EQUATIONS__

cq_status custom_wf(double *samples, double *values, int num_samples) {
    assert(samples != NULL);
    assert(values != NULL);
    assert(num_samples > 0);

    double sample_rate = get_device_sample_rate();
    double min_pulse_duration = get_device_min_pulse_duration();
    double max_pulse_duration = get_device_max_pulse_duration();

    if (num_samples / sample_rate > max_pulse_duration ||
        num_samples / sample_rate < min_pulse_duration) {
        printf("Error: The resulting custom waveform has duration not "
               "supported by device.\n"
               "Pulse duration should fall within [%f, %f] ns "
               "given %f\n",
               min_pulse_duration,
               max_pulse_duration,
               (double)num_samples / sample_rate);
        return CQ_ERROR;
    }

    for (ptrdiff_t i = 0; i < num_samples; ++i) {
        samples[i] = values[i];
    }

    return CQ_SUCCESS;
}

static double sech(double x) {
    return 2.0 / (exp(x) + exp(-x));
}

cq_status sech_wf(double *samples, double duration, double amp, double sigma) {
    assert(samples != NULL);
    if (validate_duration(duration,
                          get_device_min_pulse_duration(),
                          get_device_max_pulse_duration()) == CQ_ERROR) return CQ_ERROR;

    double sample_rate = get_device_sample_rate();
    ptrdiff_t num_samples = (ptrdiff_t)(duration * sample_rate);
    if (validate_sampling(sample_rate, duration, num_samples) == CQ_ERROR) return CQ_ERROR;
    ptrdiff_t center = (ptrdiff_t)round(((double)num_samples / 2.0));
    for (ptrdiff_t i = 0; i < num_samples; ++i) {
        samples[i] = amp * sech((double)(i - center) / sigma);
    }

    return CQ_SUCCESS;
}

cq_status sin_wf(double *samples, double duration, double amp,
                 double freq, double phase) {
    // sin is rescaled to be in [0, 1] range (if amp == 1.0)
    assert(samples != NULL);
    if (validate_duration(duration,
                          get_device_min_pulse_duration(),
                          get_device_max_pulse_duration()) == CQ_ERROR) return CQ_ERROR;

    double sample_rate = get_device_sample_rate();
    ptrdiff_t num_samples = (ptrdiff_t)(duration * sample_rate);
    if (validate_sampling(sample_rate, duration, num_samples) == CQ_ERROR) return CQ_ERROR;

    for (ptrdiff_t i = 0; i < num_samples; ++i) {
        samples[i] = amp * (0.5 * (1.0 + sin(2 * M_PI * freq * (double)i/(double)num_samples + phase)));
    }

    return CQ_SUCCESS;
}

cq_status cos_wf(double *samples, double duration, double amp,
                 double freq, double phase) {
    // cos is rescaled to be in [0, 1] range (if amp == 1.0)
    assert(samples != NULL);
    if (validate_duration(duration,
                          get_device_min_pulse_duration(),
                          get_device_max_pulse_duration()) == CQ_ERROR) return CQ_ERROR;

    double sample_rate = get_device_sample_rate();
    ptrdiff_t num_samples = (ptrdiff_t)(duration * sample_rate);
    if (validate_sampling(sample_rate, duration, num_samples) == CQ_ERROR) return CQ_ERROR;

    for (ptrdiff_t i = 0; i < num_samples; ++i) {
        samples[i] = amp * (0.5 * (1.0 + cos(2 * M_PI * freq * (double)i/(double)num_samples + phase)));
    }

    return CQ_SUCCESS;
}

cq_status blackman_wf(double *samples, double duration, double amp) {
    assert(samples != NULL);
    if (validate_duration(duration,
                          get_device_min_pulse_duration(),
                          get_device_max_pulse_duration()) == CQ_ERROR) return CQ_ERROR;

    double sample_rate = get_device_sample_rate();
    ptrdiff_t num_samples = (ptrdiff_t)(duration * sample_rate);
    if (validate_sampling(sample_rate, duration, num_samples) == CQ_ERROR) return CQ_ERROR;

#define BLACKMAN_A0_COEFF 0.42659
#define BLACKMAN_A1_COEFF 0.49656
#define BLACKMAN_A2_COEFF 0.076849

    for (ptrdiff_t i = 0; i < num_samples; ++i) {
        double ratio = (double)i / (double)num_samples;
        samples[i] = amp * (BLACKMAN_A0_COEFF -
                            BLACKMAN_A1_COEFF * cos(2 * M_PI * ratio) +
                            BLACKMAN_A2_COEFF * cos(4 * M_PI * ratio));
    }

#undef BLACKMAN_A0_COEFF
#undef BLACKMAN_A1_COEFF
#undef BLACKMAN_A2_COEFF

    return CQ_SUCCESS;
}

static double repeater(double x, double freq, double phase) {
    return x * freq + (phase / 2 * M_PI);
}

cq_status saw_wf(double *samples, double duration, double amp,
                 double freq, double phase) {
    assert(samples != NULL);
    if (validate_duration(duration,
                          get_device_min_pulse_duration(),
                          get_device_max_pulse_duration()) == CQ_ERROR) return CQ_ERROR;

    double sample_rate = get_device_sample_rate();
    ptrdiff_t num_samples = (ptrdiff_t)(duration * sample_rate);
    if (validate_sampling(sample_rate, duration, num_samples) == CQ_ERROR) return CQ_ERROR;

    for (ptrdiff_t i = 0; i < num_samples; ++i) {
        double x = (double)i / (double)num_samples;
        double wav = repeater(x, freq, phase);
        samples[i] = 2 * amp * (wav - floor(wav + 0.5));
    }

    return CQ_SUCCESS;
}

cq_status composite_wf(double *samples, double **waveforms,
                       ptrdiff_t *num_samples, int num_waveforms,
                       ptrdiff_t *total_num_samples) {
    assert(samples != NULL);
    assert(waveforms != NULL);
    assert(num_samples != NULL);
    assert(num_waveforms > 0);
    assert(total_num_samples != NULL);

    double sample_rate = get_device_sample_rate();
    double min_pulse_duration = get_device_min_pulse_duration();
    double max_pulse_duration = get_device_max_pulse_duration();

    ptrdiff_t k = 0;
    for (ptrdiff_t i = 0; i < num_waveforms; ++i) {
        for (ptrdiff_t j = 0; j < num_samples[i]; ++j) {
            samples[k] = waveforms[i][j];
            ++k;
        }
    }


    *total_num_samples = k;
    if ((double)k / sample_rate > max_pulse_duration ||
        (double)k / sample_rate < min_pulse_duration) {
        printf("Error: The resulting composite waveform has duration not "
               "supported by device.\n"
               "Pulse duration should fall within [%f, %f] ns "
               "given %f\n",
               min_pulse_duration,
               max_pulse_duration,
               (double)k / sample_rate);

        return CQ_ERROR;
    }

    return CQ_SUCCESS;
}

cq_status validate_duration(double duration, double min_duration, double max_duration) {
    if (duration < min_duration ||
        duration > max_duration) {
        printf("Error: Pulse duration should fall within [%f, %f] ns "
               "given %f\n",
               min_duration,
               max_duration,
               duration);
        return CQ_ERROR;
    }

    return CQ_SUCCESS;
}

cq_status validate_sampling(double sample_rate, double duration, ptrdiff_t num_samples) {
    if (((duration * sample_rate) - (double)num_samples) > __CQ_ANALOG_EPSILON__) {
        printf("Warning: Requested pulse duration "
               "produces non-integer number of samples. "
               "Proceeding, but sampling errors are expected.\n");
        return CQ_WARNING;
    }

    if (num_samples < 1 || num_samples > __CQ_ANALOG_MAX_NUM_SAMPLES__) {
        printf("Error: Given waveform requires too many samples. "
               "Current maximum number of samples is: %d\n",
               __CQ_ANALOG_MAX_NUM_SAMPLES__);
        return CQ_ERROR;
    }

    return CQ_SUCCESS;
}
