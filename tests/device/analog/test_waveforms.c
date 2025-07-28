#include "test_waveforms.h"
#include "analog.h"
#include "unity.h"

void setUp(void) {
    cq_enable_analog_mode(ISING);
    return;
}

void tearDown(void) {
    return;
}

void test_cq_gaussian_wf(void) {
    double expected[100] = {
        5.31236408e-242, 1.90443622e-232, 4.37749104e-223, 6.45155387e-214,
        6.09654271e-205, 3.69388307e-196, 1.43503633e-187, 3.57456238e-179,
        5.70904011e-171, 5.84633286e-163, 3.83870035e-155, 1.61608841e-147,
        4.36240770e-140, 7.55035926e-133, 8.37894253e-126, 5.96198717e-119,
        2.72002624e-112, 7.95674389e-106, 1.49237476e-099, 1.79473628e-093,
        1.38389653e-087, 6.84205918e-082, 2.16895361e-076, 4.40853133e-071,
        5.74536772e-066, 4.80089224e-061, 2.57220937e-056, 8.83630922e-052,
        1.94632663e-047, 2.74878501e-043, 2.48912125e-039, 1.44521201e-035,
        5.38018616e-032, 1.28423137e-028, 1.96548382e-025, 1.92874985e-022,
        1.21356367e-019, 4.89586526e-017, 1.26641655e-014, 2.10040929e-012,
        2.23363144e-010, 1.52299797e-008, 6.65836147e-007, 1.86644691e-005,
        3.35462628e-004, 3.86592014e-003, 2.85655008e-002, 1.35335283e-001,
        4.11112291e-001, 8.00737403e-001, 1.00000000e+000, 8.00737403e-001,
        4.11112291e-001, 1.35335283e-001, 2.85655008e-002, 3.86592014e-003,
        3.35462628e-004, 1.86644691e-005, 6.65836147e-007, 1.52299797e-008,
        2.23363144e-010, 2.10040929e-012, 1.26641655e-014, 4.89586526e-017,
        1.21356367e-019, 1.92874985e-022, 1.96548382e-025, 1.28423137e-028,
        5.38018616e-032, 1.44521201e-035, 2.48912125e-039, 2.74878501e-043,
        1.94632663e-047, 8.83630922e-052, 2.57220937e-056, 4.80089224e-061,
        5.74536772e-066, 4.40853133e-071, 2.16895361e-076, 6.84205918e-082,
        1.38389653e-087, 1.79473628e-093, 1.49237476e-099, 7.95674389e-106,
        2.72002624e-112, 5.96198717e-119, 8.37894253e-126, 7.55035926e-133,
        4.36240770e-140, 1.61608841e-147, 3.83870035e-155, 5.84633286e-163,
        5.70904011e-171, 3.57456238e-179, 1.43503633e-187, 3.69388307e-196,
        6.09654271e-205, 6.45155387e-214, 4.37749104e-223, 1.90443622e-232};

    double samples[100] = {0};
    double duration = 313.0;
    double amp = 1.0;
    double sigma = 1.5;
    double epsilon = 0.0000001;

    TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, cq_gaussian_wf(samples, duration, amp, sigma));

    TEST_ASSERT_EQUAL_INT(CQ_ERROR, cq_gaussian_wf(NULL, duration, amp, sigma));

    double negative_duration = -1.0;
    TEST_ASSERT_EQUAL_INT(CQ_ERROR,
                          cq_gaussian_wf(samples, negative_duration, amp, sigma));


    double too_short_duration = 1.0;
    TEST_ASSERT_EQUAL_INT(CQ_ERROR,
                          cq_gaussian_wf(samples, too_short_duration, amp, sigma));

    double too_long_duration = 4000000001.0;
    TEST_ASSERT_EQUAL_INT(CQ_ERROR,
                          cq_gaussian_wf(samples, too_long_duration, amp, sigma));

    duration = 400.0;
    TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, cq_gaussian_wf(samples, duration, amp, sigma));

    double sample_rate = 0.25;
    ptrdiff_t num_samples = (ptrdiff_t)(duration * sample_rate);
    for (ptrdiff_t i = 0; i < num_samples; ++i) {
        TEST_ASSERT(samples[i] - expected[i] < epsilon);
    }
}

void test_cq_gaussian_sqr_wf(void) {
    double samples[100] = {0};
    double duration = 313.0;
    double amp = 1.0;
    double sigma = 1.5;
    double width = 100.5;
    double epsilon = 0.0000001;

    TEST_ASSERT_EQUAL_INT(CQ_ERROR,
                          cq_gaussian_sqr_wf(
                              samples, duration, amp, sigma, width));
}

void test_cq_interpolated_wf(void) {
    double expected[100] = {
        -5.000000, -4.898990, -4.797980, -4.696970, -4.595960,
        -4.494949, -4.393939, -4.292929, -4.191919, -4.090909,
        -3.989899, -3.888889, -3.787879, -3.686869, -3.585859,
        -3.484848, -3.383838, -3.282828, -3.181818, -3.080808,
        -2.979798, -2.878788, -2.777778, -2.676768, -2.575758,
        -2.474747, -2.373737, -2.272727, -2.171717, -2.070707,
        -1.969697, -1.868687, -1.767677, -1.666667, -1.565657,
        -1.464646, -1.363636, -1.262626, -1.161616, -1.060606,
        -0.959596, -0.858586, -0.757576, -0.656566, -0.555556,
        -0.454545, -0.353535, -0.252525, -0.151515, -0.050505,
        0.050505, 0.151515, 0.252525, 0.353535, 0.454545,
        0.555556, 0.656566, 0.757576, 0.858586, 0.959596,
        1.060606, 1.161616, 1.262626, 1.363636, 1.464646,
        1.565657, 1.666667, 1.767677, 1.868687, 1.969697,
        2.070707, 2.171717, 2.272727, 2.373737, 2.474747,
        2.575758, 2.676768, 2.777778, 2.878788, 2.979798,
        3.080808, 3.181818, 3.282828, 3.383838, 3.484848,
        3.585859, 3.686869, 3.787879, 3.888889, 3.989899,
        4.090909, 4.191919, 4.292929, 4.393939, 4.494949,
        4.595960, 4.696970, 4.797980, 4.898990, 5.000000
    };

    double expected2[100] = {
        0.000000, 0.095081, 0.190084, 0.284933, 0.379548,
        0.473853, 0.567769, 0.661220, 0.754128, 0.846415,
        0.938003, 1.028815, 1.118773, 1.207799, 1.295817,
        1.382748, 1.468515, 1.553039, 1.636244, 1.718052,
        1.798386, 1.877167, 1.954317, 2.029761, 2.103419,
        2.175214, 2.245068, 2.312905, 2.378645, 2.442213,
        2.503529, 2.562517, 2.619099, 2.673196, 2.724733,
        2.773630, 2.819810, 2.863197, 2.903711, 2.941276,
        2.975813, 3.007246, 3.035496, 3.060487, 3.082139,
        3.100377, 3.115121, 3.126295, 3.133821, 3.137621,
        3.137621, 3.133821, 3.126295, 3.115121, 3.100377,
        3.082139, 3.060487, 3.035496, 3.007246, 2.975813,
        2.941276, 2.903711, 2.863197, 2.819810, 2.773630,
        2.724733, 2.673196, 2.619099, 2.562517, 2.503529,
        2.442213, 2.378645, 2.312905, 2.245068, 2.175214,
        2.103419, 2.029761, 1.954317, 1.877167, 1.798386,
        1.718052, 1.636244, 1.553039, 1.468515, 1.382748,
        1.295817, 1.207799, 1.118773, 1.028815, 0.938003,
        0.846415, 0.754128, 0.661220, 0.567769, 0.473853,
        0.379548, 0.284933, 0.190084, 0.095081, 0.000000
    };

    double samples[100] = {0};
    double duration = 400.0;
    double points[3] = {-5.0, 0.0, 5.0};
    int num_points = 3;
    double epsilon = 0.00001;

    TEST_ASSERT_EQUAL_INT(CQ_ERROR,
                          cq_interpolated_wf(
                              NULL, duration, points, num_points));

    TEST_ASSERT_EQUAL_INT(CQ_ERROR,
                          cq_interpolated_wf(
                              samples, duration, NULL, num_points));

    double negative_duration = -1.0;
    TEST_ASSERT_EQUAL_INT(CQ_ERROR,
                          cq_interpolated_wf(
                              samples, negative_duration, points, num_points));

    double too_short_duration = 5.0;
    TEST_ASSERT_EQUAL_INT(CQ_ERROR,
                          cq_interpolated_wf(
                              samples, too_short_duration, points, num_points));

    double too_long_duration = 1231411.0;
    TEST_ASSERT_EQUAL_INT(CQ_ERROR,
                          cq_interpolated_wf(
                              samples, too_long_duration, points, num_points));

    int neg_num_points = -42;
    TEST_ASSERT_EQUAL_INT(CQ_ERROR,
                          cq_interpolated_wf(
                              samples, duration, points, neg_num_points));
    int too_few_points = 1;
    TEST_ASSERT_EQUAL_INT(CQ_ERROR,
                          cq_interpolated_wf(
                              samples, duration, points, too_few_points));
    int too_many_points = 67;
    TEST_ASSERT_EQUAL_INT(CQ_ERROR,
                          cq_interpolated_wf(
                              samples, duration, points, too_many_points));

    TEST_ASSERT_EQUAL_INT(CQ_SUCCESS,
                          cq_interpolated_wf(
                              samples, duration, points, num_points));
    double sample_rate = 0.25;
    ptrdiff_t num_samples = (ptrdiff_t)(duration * sample_rate);

    for (ptrdiff_t i = 0; i < num_samples; ++i) {
        double result = samples[i] - expected[i];
        if (result < 0.0) result = -result;
        TEST_ASSERT(result < epsilon);
    }

    points[0] = 1e-9;
    points[1] = 3.1381;
    points[2] = 1e-9;
    cq_interpolated_wf(samples, duration, points, num_points);
    for (ptrdiff_t i = 0; i < num_samples; ++i) {
        double result = samples[i] - expected2[i];
        if (result < 0.0) result = -result;
        TEST_ASSERT(result < epsilon);
    }
}

void test_cq_sech_wf(void) {
    double expected[100] = {
         8.33276632e-11, 1.37384091e-10, 2.26508073e-10, 3.73448677e-10,
         6.15712778e-10, 1.01513875e-09, 1.67368086e-09, 2.75943323e-09,
         4.54953626e-09, 7.50091720e-09, 1.23669217e-08, 2.03896069e-08,
         3.36167786e-08, 5.54246980e-08, 9.13798785e-08, 1.50659949e-07,
         2.48396263e-07, 4.09536203e-07, 6.75211048e-07, 1.11323482e-06,
         1.83541392e-06, 3.02608598e-06, 4.98917231e-06, 8.22575452e-06,
         1.35619764e-05, 2.23599190e-05, 3.68652741e-05, 6.07805616e-05,
         1.00210205e-04, 1.65218696e-04, 2.72399578e-04, 4.49110977e-04,
         7.40458813e-04, 1.22081016e-03, 2.01277554e-03, 3.31850521e-03,
         5.47128724e-03, 9.02061477e-03, 1.48724217e-02, 2.45202191e-02,
         4.04258467e-02, 6.66457545e-02, 1.09856980e-01, 1.81019232e-01,
         2.97983782e-01, 4.89213696e-01, 7.97406687e-01, 1.27528810e+00,
         1.94416282e+00, 2.66045665e+00, 3.00000000e+00, 2.66045665e+00,
         1.94416282e+00, 1.27528810e+00, 7.97406687e-01, 4.89213696e-01,
         2.97983782e-01, 1.81019232e-01, 1.09856980e-01, 6.66457545e-02,
         4.04258467e-02, 2.45202191e-02, 1.48724217e-02, 9.02061477e-03,
         5.47128724e-03, 3.31850521e-03, 2.01277554e-03, 1.22081016e-03,
         7.40458813e-04, 4.49110977e-04, 2.72399578e-04, 1.65218696e-04,
         1.00210205e-04, 6.07805616e-05, 3.68652741e-05, 2.23599190e-05,
         1.35619764e-05, 8.22575452e-06, 4.98917231e-06, 3.02608598e-06,
         1.83541392e-06, 1.11323482e-06, 6.75211048e-07, 4.09536203e-07,
         2.48396263e-07, 1.50659949e-07, 9.13798785e-08, 5.54246980e-08,
         3.36167786e-08, 2.03896069e-08, 1.23669217e-08, 7.50091720e-09,
         4.54953626e-09, 2.75943323e-09, 1.67368086e-09, 1.01513875e-09,
         6.15712778e-10, 3.73448677e-10, 2.26508073e-10, 1.37384091e-10
    };

    double samples[100] = {0};
    double duration = 312.0;
    double amp = 3.0;
    double sigma = 2.0;
    double epsilon = 0.0000001;

    TEST_ASSERT_EQUAL_INT(CQ_ERROR,
                          cq_sech_wf(NULL, duration, amp, sigma));

    double negative_duration = -1.0;
    TEST_ASSERT_EQUAL_INT(CQ_ERROR,
                          cq_sech_wf(samples, negative_duration, amp, sigma));

    double too_short_duration = 5.0;
    TEST_ASSERT_EQUAL_INT(CQ_ERROR,
                          cq_sech_wf(samples, too_short_duration, amp, sigma));

    double too_long_duration = 1231411.0;
    TEST_ASSERT_EQUAL_INT(CQ_ERROR,
                          cq_sech_wf(samples, too_long_duration, amp, sigma));

    duration = 400.0;
    TEST_ASSERT_EQUAL_INT(CQ_SUCCESS,
                          cq_sech_wf(samples, duration, amp, sigma));

    double sample_rate = 0.25;
    ptrdiff_t num_samples = (ptrdiff_t)(duration * sample_rate);
    for (ptrdiff_t i = 0; i < num_samples; ++i) {
        TEST_ASSERT(samples[i] - expected[i] < epsilon);
    }
}

void test_cq_sin_wf(void) {
    double expected[100] = {
          0.00000000e+00,  1.25333234e-01,  2.48689887e-01,  3.68124553e-01,
          4.81753674e-01,  5.87785252e-01,  6.84547106e-01,  7.70513243e-01,
          8.44327926e-01,  9.04827052e-01,  9.51056516e-01,  9.82287251e-01,
          9.98026728e-01,  9.98026728e-01,  9.82287251e-01,  9.51056516e-01,
          9.04827052e-01,  8.44327926e-01,  7.70513243e-01,  6.84547106e-01,
          5.87785252e-01,  4.81753674e-01,  3.68124553e-01,  2.48689887e-01,
          1.25333234e-01,  1.22464680e-16, -1.25333234e-01, -2.48689887e-01,
         -3.68124553e-01, -4.81753674e-01, -5.87785252e-01, -6.84547106e-01,
         -7.70513243e-01, -8.44327926e-01, -9.04827052e-01, -9.51056516e-01,
         -9.82287251e-01, -9.98026728e-01, -9.98026728e-01, -9.82287251e-01,
         -9.51056516e-01, -9.04827052e-01, -8.44327926e-01, -7.70513243e-01,
         -6.84547106e-01, -5.87785252e-01, -4.81753674e-01, -3.68124553e-01,
         -2.48689887e-01, -1.25333234e-01, -2.44929360e-16,  1.25333234e-01,
          2.48689887e-01,  3.68124553e-01,  4.81753674e-01,  5.87785252e-01,
          6.84547106e-01,  7.70513243e-01,  8.44327926e-01,  9.04827052e-01,
          9.51056516e-01,  9.82287251e-01,  9.98026728e-01,  9.98026728e-01,
          9.82287251e-01,  9.51056516e-01,  9.04827052e-01,  8.44327926e-01,
          7.70513243e-01,  6.84547106e-01,  5.87785252e-01,  4.81753674e-01,
          3.68124553e-01,  2.48689887e-01,  1.25333234e-01,  3.67394040e-16,
         -1.25333234e-01, -2.48689887e-01, -3.68124553e-01, -4.81753674e-01,
         -5.87785252e-01, -6.84547106e-01, -7.70513243e-01, -8.44327926e-01,
         -9.04827052e-01, -9.51056516e-01, -9.82287251e-01, -9.98026728e-01,
         -9.98026728e-01, -9.82287251e-01, -9.51056516e-01, -9.04827052e-01,
         -8.44327926e-01, -7.70513243e-01, -6.84547106e-01, -5.87785252e-01,
         -4.81753674e-01, -3.68124553e-01, -2.48689887e-01, -1.25333234e-01,
    };

    double samples[100] = {0};
    double duration = 313.0;
    double amp = 1.0;
    double freq = 2.0;
    double phase = 0.0;
    double epsilon = 0.0000001;

    TEST_ASSERT_EQUAL_INT(CQ_SUCCESS,
                          cq_sin_wf(samples, duration, amp, freq, phase));

    TEST_ASSERT_EQUAL_INT(CQ_ERROR,
                          cq_sin_wf(NULL, duration, amp, freq, phase));

    double negative_duration = -1.0;
    TEST_ASSERT_EQUAL_INT(CQ_ERROR,
                          cq_sin_wf(samples, negative_duration, amp, freq, phase));

    double too_short_duration = 1.0;
    TEST_ASSERT_EQUAL_INT(CQ_ERROR,
                          cq_sin_wf(samples, too_short_duration, amp, freq, phase));

    double too_long_duration = 4000000001.0;
    TEST_ASSERT_EQUAL_INT(CQ_ERROR,
                          cq_sin_wf(samples, too_long_duration, amp, freq, phase));

    double negative_freq = -1.0;
    TEST_ASSERT_EQUAL_INT(CQ_ERROR,
                          cq_sin_wf(samples, duration, amp, negative_freq, phase));

    duration = 400.0;
    TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, cq_sin_wf(samples, duration, amp, freq, phase));

    double sample_rate = 0.25;
    ptrdiff_t num_samples = (ptrdiff_t)(duration * sample_rate);
    for (ptrdiff_t i = 0; i < num_samples; ++i) {
        TEST_ASSERT(samples[i] - expected[i] < epsilon);
    }
}

void test_cq_cos_wf(void) {
    double expected[100] = {
      1.        ,  0.9921147 ,  0.96858316,  0.92977649,  0.87630668,  0.80901699,
      0.72896863,  0.63742399,  0.53582679,  0.42577929,  0.30901699,  0.18738131,
      0.06279052, -0.06279052, -0.18738131, -0.30901699, -0.42577929, -0.53582679,
     -0.63742399, -0.72896863, -0.80901699, -0.87630668, -0.92977649, -0.96858316,
     -0.9921147 , -1.        , -0.9921147 , -0.96858316, -0.92977649, -0.87630668,
     -0.80901699, -0.72896863, -0.63742399, -0.53582679, -0.42577929, -0.30901699,
     -0.18738131, -0.06279052,  0.06279052,  0.18738131,  0.30901699,  0.42577929,
      0.53582679,  0.63742399,  0.72896863,  0.80901699,  0.87630668,  0.92977649,
      0.96858316,  0.9921147 ,  1.        ,  0.9921147 ,  0.96858316,  0.92977649,
      0.87630668,  0.80901699,  0.72896863,  0.63742399,  0.53582679,  0.42577929,
      0.30901699,  0.18738131,  0.06279052, -0.06279052, -0.18738131, -0.30901699,
     -0.42577929, -0.53582679, -0.63742399, -0.72896863, -0.80901699, -0.87630668,
     -0.92977649, -0.96858316, -0.9921147 , -1.        , -0.9921147 , -0.96858316,
     -0.92977649, -0.87630668, -0.80901699, -0.72896863, -0.63742399, -0.53582679,
     -0.42577929, -0.30901699, -0.18738131, -0.06279052,  0.06279052,  0.18738131,
      0.30901699,  0.42577929,  0.53582679,  0.63742399,  0.72896863,  0.80901699,
      0.87630668,  0.92977649,  0.96858316,  0.9921147
    };
    double samples[100] = {0};
    double duration = 313.0;
    double amp = 1.0;
    double freq = 2.0;
    double phase = 0.0;
    double epsilon = 0.0000001;

    TEST_ASSERT_EQUAL_INT(CQ_SUCCESS,
                          cq_cos_wf(samples, duration, amp, freq, phase));

    TEST_ASSERT_EQUAL_INT(CQ_ERROR,
                          cq_cos_wf(NULL, duration, amp, freq, phase));

    double negative_duration = -1.0;
    TEST_ASSERT_EQUAL_INT(CQ_ERROR,
                          cq_cos_wf(samples, negative_duration, amp, freq, phase));

    double too_short_duration = 1.0;
    TEST_ASSERT_EQUAL_INT(CQ_ERROR,
                          cq_cos_wf(samples, too_short_duration, amp, freq, phase));

    double too_long_duration = 4000000001.0;
    TEST_ASSERT_EQUAL_INT(CQ_ERROR,
                          cq_cos_wf(samples, too_long_duration, amp, freq, phase));

    double negative_freq = -1.0;
    TEST_ASSERT_EQUAL_INT(CQ_ERROR,
                          cq_cos_wf(samples, duration, amp, negative_freq, phase));

    duration = 400.0;
    TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, cq_cos_wf(samples, duration, amp, freq, phase));

    double sample_rate = 0.25;
    ptrdiff_t num_samples = (ptrdiff_t)(duration * sample_rate);
    for (ptrdiff_t i = 0; i < num_samples; ++i) {
        TEST_ASSERT(samples[i] - expected[i] < epsilon);
    }
}

void test_cq_blackman_wf(void) {
    double expected[100] = {
         0.006879   , 0.00725287 , 0.00838017 , 0.01027784 , 0.01297364 , 0.01650552,
         0.0209207  , 0.02627448 , 0.03262891 , 0.04005124 , 0.04861217 , 0.05838401,
         0.06943873 , 0.0818459  , 0.09567068 , 0.11097171 , 0.12779913 , 0.14619264,
         0.16617964 , 0.18777356 , 0.21097237 , 0.23575726 , 0.26209154 , 0.28991988,
         0.31916772 , 0.349741   , 0.38152624 , 0.41439082 , 0.44818367 , 0.48273616,
         0.51786333 , 0.55336542 , 0.58902957 , 0.62463185 , 0.65993944 , 0.694713  ,
         0.72870919 , 0.76168332 , 0.79339205 , 0.82359612 , 0.85206313 , 0.87857019,
         0.9029066  , 0.92487632 , 0.94430032 , 0.96101877 , 0.97489295 , 0.98580695,
         0.99366912 , 0.99841317 , 0.999999   , 0.99841317 , 0.99366912 , 0.98580695,
         0.97489295 , 0.96101877 , 0.94430032 , 0.92487632 , 0.9029066  , 0.87857019,
         0.85206313 , 0.82359612 , 0.79339205 , 0.76168332 , 0.72870919 , 0.694713  ,
         0.65993944 , 0.62463185 , 0.58902957 , 0.55336542 , 0.51786333 , 0.48273616,
         0.44818367 , 0.41439082 , 0.38152624 , 0.349741   , 0.31916772 , 0.28991988,
         0.26209154 , 0.23575726 , 0.21097237 , 0.18777356 , 0.16617964 , 0.14619264,
         0.12779913 , 0.11097171 , 0.09567068 , 0.0818459  , 0.06943873 , 0.05838401,
         0.04861217 , 0.04005124 , 0.03262891 , 0.02627448 , 0.0209207  , 0.01650552,
         0.01297364 , 0.01027784 , 0.00838017 , 0.00725287
    };
    double samples[100] = {0};
    double duration = 313.0;
    double amp = 1.0;
    double freq = 2.0;
    double phase = 0.0;
    double epsilon = 0.0000001;

    TEST_ASSERT_EQUAL_INT(CQ_SUCCESS,
                          cq_blackman_wf(samples, duration, amp));

    TEST_ASSERT_EQUAL_INT(CQ_ERROR,
                          cq_blackman_wf(NULL, duration, amp));

    double negative_duration = -1.0;
    TEST_ASSERT_EQUAL_INT(CQ_ERROR,
                          cq_blackman_wf(samples, negative_duration, amp));

    double too_short_duration = 1.0;
    TEST_ASSERT_EQUAL_INT(CQ_ERROR,
                          cq_blackman_wf(samples, too_short_duration, amp));

    double too_long_duration = 4000000001.0;
    TEST_ASSERT_EQUAL_INT(CQ_ERROR,
                          cq_blackman_wf(samples, too_long_duration, amp));

    duration = 400.0;
    TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, cq_blackman_wf(samples, duration, amp));

    double sample_rate = 0.25;
    ptrdiff_t num_samples = (ptrdiff_t)(duration * sample_rate);
    for (ptrdiff_t i = 0; i < num_samples; ++i) {
        TEST_ASSERT(samples[i] - expected[i] < epsilon);
    }
}

void test_cq_saw_wf(void) {
    double expected[100] = {
          0.,    0.04,  0.08,  0.12,  0.16,  0.2 ,  0.24,  0.28,
          0.32,  0.36,  0.4 ,  0.44,  0.48,  0.52,  0.56,  0.6 ,
          0.64,  0.68,  0.72,  0.76,  0.8 ,  0.84,  0.88,  0.92,
          0.96, -1.  , -0.96, -0.92, -0.88, -0.84, -0.8 , -0.76,
          -0.72, -0.68, -0.64, -0.6 ,-0.56, -0.52, -0.48, -0.44,
          -0.4 , -0.36, -0.32, -0.28, -0.24, -0.2 , -0.16, -0.12,
          -0.08, -0.04,  0.  ,  0.04,  0.08,  0.12,  0.16,  0.2 ,
          0.24,  0.28,  0.32,  0.36, 0.4 ,  0.44,  0.48,  0.52,
          0.56,  0.6 ,  0.64,  0.68,  0.72,  0.76,  0.8 ,  0.84,
          0.88,  0.92,  0.96, -1.  , -0.96, -0.92, -0.88, -0.84,
          -0.8 , -0.76, -0.72, -0.68, -0.64, -0.6 , -0.56, -0.52,
          -0.48, -0.44, -0.4 , -0.36, -0.32, -0.28, -0.24, -0.2 ,
          -0.16, -0.12, -0.08, -0.04
    };

    double samples[100] = {0};
    double duration = 313.0;
    double amp = 1.0;
    double freq = 2.0;
    double phase = 0.0;
    double epsilon = 0.0000001;

    TEST_ASSERT_EQUAL_INT(CQ_SUCCESS,
                          cq_saw_wf(samples, duration, amp, freq, phase));

    TEST_ASSERT_EQUAL_INT(CQ_ERROR,
                          cq_saw_wf(NULL, duration, amp, freq, phase));

    double negative_duration = -1.0;
    TEST_ASSERT_EQUAL_INT(CQ_ERROR,
                          cq_saw_wf(samples, negative_duration, amp, freq, phase));

    double too_short_duration = 1.0;
    TEST_ASSERT_EQUAL_INT(CQ_ERROR,
                          cq_saw_wf(samples, too_short_duration, amp, freq, phase));

    double too_long_duration = 4000000001.0;
    TEST_ASSERT_EQUAL_INT(CQ_ERROR,
                          cq_saw_wf(samples, too_long_duration, amp, freq, phase));

    double negative_freq = -1.0;
    TEST_ASSERT_EQUAL_INT(CQ_ERROR,
                          cq_saw_wf(samples, duration, amp, negative_freq, phase));

    duration = 400.0;
    TEST_ASSERT_EQUAL_INT(CQ_SUCCESS,
                          cq_saw_wf(samples, duration, amp, freq, phase));

    double sample_rate = 0.25;
    ptrdiff_t num_samples = (ptrdiff_t)(duration * sample_rate);
    for (ptrdiff_t i = 0; i < num_samples; ++i) {
        TEST_ASSERT(samples[i] - expected[i] < epsilon);
    }
}

void test_cq_custom_wf(void) {
    double samples[100] = {0};
    double values[100] = {
        0 , 1 , 2 , 3 , 4 , 5 , 6 , 7 , 8 , 9 , 10,
        11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
        21, 22, 23, 24, 25, 26, 27, 28, 29, 30,
        31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
        41, 42, 43, 44, 45, 46, 47, 48, 49, 50,
        51, 52, 53, 54, 55, 56, 57, 58, 59, 60,
        61, 62, 63, 64, 65, 66, 67, 68, 69, 70,
        71, 72, 73, 74, 75, 76, 77, 78, 79, 80,
        81, 82, 83, 84, 85, 86, 87, 88, 89, 90,
        91, 92, 93, 94, 95, 96, 97, 98, 99
    };
    double epsilon = 0.0000001;
    int num_samples = 100;

    TEST_ASSERT_EQUAL_INT(CQ_ERROR,
                          cq_custom_wf(NULL, values, num_samples));

    TEST_ASSERT_EQUAL_INT(CQ_ERROR,
                          cq_custom_wf(samples, NULL, num_samples));


    double values_diff_size[4] = {0, 1, 2, 3};
    int values_size = 4;
     TEST_ASSERT_EQUAL_INT(CQ_SUCCESS,
                          cq_custom_wf(samples, values_diff_size, values_size));


    int neg_num_samples = -1;
    TEST_ASSERT_EQUAL_INT(CQ_ERROR,
                          cq_custom_wf(samples, values, neg_num_samples));

    int too_few_samples = 1;
    TEST_ASSERT_EQUAL_INT(CQ_ERROR,
                          cq_custom_wf(samples, values, too_few_samples));

    ptrdiff_t too_many_samples = 124444444444;
    TEST_ASSERT_EQUAL_INT(CQ_ERROR,
                          cq_custom_wf(samples, values, too_many_samples));

    TEST_ASSERT_EQUAL_INT(CQ_SUCCESS,
                          cq_custom_wf(samples, values, num_samples));

    for (ptrdiff_t i = 0; i < num_samples; ++i) {
        TEST_ASSERT(samples[i] - values[i] < epsilon);
    }
}
void test_cq_composite_wf(void) {
    double expected[300] = {
      5.31236408e-242,  1.90443622e-232,  4.37749104e-223,  6.45155387e-214,
      6.09654271e-205,  3.69388307e-196,  1.43503633e-187,  3.57456238e-179,
      5.70904011e-171,  5.84633286e-163,  3.83870035e-155,  1.61608841e-147,
      4.36240770e-140,  7.55035926e-133,  8.37894253e-126,  5.96198717e-119,
      2.72002624e-112,  7.95674389e-106,  1.49237476e-099,  1.79473628e-093,
      1.38389653e-087,  6.84205918e-082,  2.16895361e-076,  4.40853133e-071,
      5.74536772e-066,  4.80089224e-061,  2.57220937e-056,  8.83630922e-052,
      1.94632663e-047,  2.74878501e-043,  2.48912125e-039,  1.44521201e-035,
      5.38018616e-032,  1.28423137e-028,  1.96548382e-025,  1.92874985e-022,
      1.21356367e-019,  4.89586526e-017,  1.26641655e-014,  2.10040929e-012,
      2.23363144e-010,  1.52299797e-008,  6.65836147e-007,  1.86644691e-005,
      3.35462628e-004,  3.86592014e-003,  2.85655008e-002,  1.35335283e-001,
      4.11112291e-001,  8.00737403e-001,  1.00000000e+000,  8.00737403e-001,
      4.11112291e-001,  1.35335283e-001,  2.85655008e-002,  3.86592014e-003,
      3.35462628e-004,  1.86644691e-005,  6.65836147e-007,  1.52299797e-008,
      2.23363144e-010,  2.10040929e-012,  1.26641655e-014,  4.89586526e-017,
      1.21356367e-019,  1.92874985e-022,  1.96548382e-025,  1.28423137e-028,
      5.38018616e-032,  1.44521201e-035,  2.48912125e-039,  2.74878501e-043,
      1.94632663e-047,  8.83630922e-052,  2.57220937e-056,  4.80089224e-061,
      5.74536772e-066,  4.40853133e-071,  2.16895361e-076,  6.84205918e-082,
      1.38389653e-087,  1.79473628e-093,  1.49237476e-099,  7.95674389e-106,
      2.72002624e-112,  5.96198717e-119,  8.37894253e-126,  7.55035926e-133,
      4.36240770e-140,  1.61608841e-147,  3.83870035e-155,  5.84633286e-163,
      5.70904011e-171,  3.57456238e-179,  1.43503633e-187,  3.69388307e-196,
      6.09654271e-205,  6.45155387e-214,  4.37749104e-223,  1.90443622e-232,
      0.00000000e+000,  1.25333234e-001,  2.48689887e-001,  3.68124553e-001,
      4.81753674e-001,  5.87785252e-001,  6.84547106e-001,  7.70513243e-001,
      8.44327926e-001,  9.04827052e-001,  9.51056516e-001,  9.82287251e-001,
      9.98026728e-001,  9.98026728e-001,  9.82287251e-001,  9.51056516e-001,
      9.04827052e-001,  8.44327926e-001,  7.70513243e-001,  6.84547106e-001,
      5.87785252e-001,  4.81753674e-001,  3.68124553e-001,  2.48689887e-001,
      1.25333234e-001,  1.22464680e-016, -1.25333234e-001, -2.48689887e-001,
     -3.68124553e-001, -4.81753674e-001, -5.87785252e-001, -6.84547106e-001,
     -7.70513243e-001, -8.44327926e-001, -9.04827052e-001, -9.51056516e-001,
     -9.82287251e-001, -9.98026728e-001, -9.98026728e-001, -9.82287251e-001,
     -9.51056516e-001, -9.04827052e-001, -8.44327926e-001, -7.70513243e-001,
     -6.84547106e-001, -5.87785252e-001, -4.81753674e-001, -3.68124553e-001,
     -2.48689887e-001, -1.25333234e-001, -2.44929360e-016,  1.25333234e-001,
      2.48689887e-001,  3.68124553e-001,  4.81753674e-001,  5.87785252e-001,
      6.84547106e-001,  7.70513243e-001,  8.44327926e-001,  9.04827052e-001,
      9.51056516e-001,  9.82287251e-001,  9.98026728e-001,  9.98026728e-001,
      9.82287251e-001,  9.51056516e-001,  9.04827052e-001,  8.44327926e-001,
      7.70513243e-001,  6.84547106e-001,  5.87785252e-001,  4.81753674e-001,
      3.68124553e-001,  2.48689887e-001,  1.25333234e-001,  3.67394040e-016,
     -1.25333234e-001, -2.48689887e-001, -3.68124553e-001, -4.81753674e-001,
     -5.87785252e-001, -6.84547106e-001, -7.70513243e-001, -8.44327926e-001,
     -9.04827052e-001, -9.51056516e-001, -9.82287251e-001, -9.98026728e-001,
     -9.98026728e-001, -9.82287251e-001, -9.51056516e-001, -9.04827052e-001,
     -8.44327926e-001, -7.70513243e-001, -6.84547106e-001, -5.87785252e-001,
     -4.81753674e-001, -3.68124553e-001, -2.48689887e-001, -1.25333234e-001,
      1.00000000e+000,  9.92114701e-001,  9.68583161e-001,  9.29776486e-001,
      8.76306680e-001,  8.09016994e-001,  7.28968627e-001,  6.37423990e-001,
      5.35826795e-001,  4.25779292e-001,  3.09016994e-001,  1.87381315e-001,
      6.27905195e-002, -6.27905195e-002, -1.87381315e-001, -3.09016994e-001,
     -4.25779292e-001, -5.35826795e-001, -6.37423990e-001, -7.28968627e-001,
     -8.09016994e-001, -8.76306680e-001, -9.29776486e-001, -9.68583161e-001,
     -9.92114701e-001, -1.00000000e+000, -9.92114701e-001, -9.68583161e-001,
     -9.29776486e-001, -8.76306680e-001, -8.09016994e-001, -7.28968627e-001,
     -6.37423990e-001, -5.35826795e-001, -4.25779292e-001, -3.09016994e-001,
     -1.87381315e-001, -6.27905195e-002,  6.27905195e-002,  1.87381315e-001,
      3.09016994e-001,  4.25779292e-001,  5.35826795e-001,  6.37423990e-001,
      7.28968627e-001,  8.09016994e-001,  8.76306680e-001,  9.29776486e-001,
      9.68583161e-001,  9.92114701e-001,  1.00000000e+000,  9.92114701e-001,
      9.68583161e-001,  9.29776486e-001,  8.76306680e-001,  8.09016994e-001,
      7.28968627e-001,  6.37423990e-001,  5.35826795e-001,  4.25779292e-001,
      3.09016994e-001,  1.87381315e-001,  6.27905195e-002, -6.27905195e-002,
     -1.87381315e-001, -3.09016994e-001, -4.25779292e-001, -5.35826795e-001,
     -6.37423990e-001, -7.28968627e-001, -8.09016994e-001, -8.76306680e-001,
     -9.29776486e-001, -9.68583161e-001, -9.92114701e-001, -1.00000000e+000,
     -9.92114701e-001, -9.68583161e-001, -9.29776486e-001, -8.76306680e-001,
     -8.09016994e-001, -7.28968627e-001, -6.37423990e-001, -5.35826795e-001,
     -4.25779292e-001, -3.09016994e-001, -1.87381315e-001, -6.27905195e-002,
      6.27905195e-002,  1.87381315e-001,  3.09016994e-001,  4.25779292e-001,
      5.35826795e-001,  6.37423990e-001,  7.28968627e-001,  8.09016994e-001,
      8.76306680e-001,  9.29776486e-001,  9.68583161e-001,  9.92114701e-001,
    };

    double gaussian_wav[100] = {0};
    double sin_wav[100] = {0};
    double cos_wav[100] = {0};
    double composite_wav[300];
    double duration = 400.0;
    double amp = 1.0;
    double sigma = 1.5;
    double freq = 2.0;
    double epsilon = 0.0000001;

    cq_gaussian_wf(gaussian_wav, duration, amp, sigma);
    cq_sin_wf(sin_wav, duration, amp, freq, 0.0);
    cq_cos_wf(cos_wav, duration, amp, freq, 0.0);
    double *waveforms[3] = {gaussian_wav, sin_wav, cos_wav};

    ptrdiff_t num_samples[3] = {100, 100, 100};
    int num_waveforms = 3;
    ptrdiff_t total_num_samples = 0;

    TEST_ASSERT_EQUAL_INT(CQ_ERROR,
                            cq_composite_wf(NULL, waveforms,
                                num_samples, num_waveforms,
                                &total_num_samples));

    TEST_ASSERT_EQUAL_INT(CQ_ERROR,
                            cq_composite_wf(composite_wav, NULL,
                                num_samples, num_waveforms,
                                &total_num_samples));

    TEST_ASSERT_EQUAL_INT(CQ_ERROR,
                            cq_composite_wf(composite_wav, waveforms,
                                NULL, num_waveforms,
                                &total_num_samples));

    TEST_ASSERT_EQUAL_INT(CQ_ERROR,
                            cq_composite_wf(composite_wav, waveforms,
                                num_samples, num_waveforms,
                                NULL));

    ptrdiff_t neg_num_samples[3] = {-24, 5, 5};
    TEST_ASSERT_EQUAL_INT(CQ_ERROR,
                            cq_composite_wf(composite_wav, waveforms,
                                neg_num_samples, num_waveforms,
                                &total_num_samples));

    int neg_num_waveforms = -1;
    TEST_ASSERT_EQUAL_INT(CQ_ERROR,
                            cq_composite_wf(composite_wav, waveforms,
                                num_samples, neg_num_waveforms,
                                &total_num_samples));



    TEST_ASSERT_EQUAL_INT(CQ_SUCCESS,
                cq_composite_wf(composite_wav, waveforms,
                                num_samples, num_waveforms,
                                &total_num_samples));

    for (ptrdiff_t i = 0; i < total_num_samples; ++i) {
        TEST_ASSERT(composite_wav[i] - expected[i] < epsilon);
    }
}


