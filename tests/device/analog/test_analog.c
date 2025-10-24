#include "unity.h"

#include "analog.h"
#include "cq.h"
#include "src/device/resources.h"
#include "src/device/control.h"

#include <stdbool.h>

#define CQ_ADDR_GLOBAL 0
#define CQ_ADDR_LOCAL 1

qubit *qr = NULL;
void setUp(void) {
    int param = 0;
    initialise_simulator(&param);
    const size_t NQUBITS = 5;
    alloc_qureg(&qr, NQUBITS);
    return;
}

void tearDown(void) {
    int param = 0;
    free_qureg(&qr);
    return;
}

void test_cq_enable_analog_mode(void) {
    TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, cq_enable_analog_mode(ISING));
    // Re-initialization should fail
    TEST_ASSERT_EQUAL_INT(CQ_ERROR, cq_enable_analog_mode(ISING));
    cq_print_analog_device();
    cq_print_avail_channels();
}

void test_cq_duration_to_samples(void) {
    double duration = 400;
    ptrdiff_t expected = 100;

    TEST_ASSERT_EQUAL_INT(expected, cq_duration_to_samples(duration));
    TEST_ASSERT_EQUAL_INT(0, cq_duration_to_samples(-42.0));
}

void test_cq_samples_to_duration(void) {
    double num_samples = 100;
    ptrdiff_t expected = 400;
    double epsilon = 0.0000001;

    TEST_ASSERT(expected - cq_samples_to_duration(num_samples) < epsilon);
    TEST_ASSERT(0.0 - cq_samples_to_duration(-12) < epsilon);
}

void test_cq_enable_analog_qreg(void) {
    // Good inputs
    int qreg_id = 0;
    int num_qubits = 2;
    TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, cq_enable_analog_qreg(qr));

    // Re-initialization should fail
    TEST_ASSERT_EQUAL_INT(CQ_ERROR, cq_enable_analog_qreg(qr));

    // Bad qr
    TEST_ASSERT_EQUAL_INT(CQ_ERROR, cq_enable_analog_qreg(NULL));

    qubit *qr2 = NULL;
    alloc_qureg(&qr2, 6);
    cq_disable_analog_qreg(qr);
    TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, cq_enable_analog_qreg(qr2));
    free_qureg(&qr2);
}

void test_cq_disable_analog_mode(void) {
    // Disabling without enabling fails
    TEST_ASSERT_EQUAL_INT(CQ_ERROR, cq_disable_analog_qreg(qr));

    // Good inputs
    cq_enable_analog_qreg(qr);
    TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, cq_disable_analog_qreg(qr));

    // Repeating on the same qreg should fail
    TEST_ASSERT_EQUAL_INT(CQ_ERROR, cq_disable_analog_qreg(qr));

    // Bad qreg_id
    TEST_ASSERT_EQUAL_INT(CQ_ERROR, cq_disable_analog_qreg(NULL));
}

void test_cq_get_channel(void) {
    channel ch0 = {0};
		  
    // Getting channel on uninitialised qreg.
    TEST_ASSERT_EQUAL_INT(CQ_ERROR,
		    cq_get_channel(&ch0, CQ_ADDR_LOCAL, qr, &qr[0]));

    cq_enable_analog_qreg(qr);
    TEST_ASSERT_EQUAL_INT(CQ_SUCCESS,
		    cq_get_channel(&ch0, CQ_ADDR_LOCAL, qr, &qr[0]));

    TEST_ASSERT_EQUAL_INT(CQ_SUCCESS,
		    cq_get_channel(&ch0, CQ_ADDR_GLOBAL, qr, NULL));

    TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, cq_print_channel(&ch0));

    // NULL channel
    TEST_ASSERT_EQUAL_INT(CQ_ERROR,
		    cq_get_channel(NULL, CQ_ADDR_LOCAL, qr, &qr[0]));
    
    // NULL qr
    TEST_ASSERT_EQUAL_INT(CQ_ERROR,
		    cq_get_channel(&ch0, CQ_ADDR_LOCAL, NULL, &qr[0]));

    // Bad type
    int negative_type = -1;
    TEST_ASSERT_EQUAL_INT(CQ_ERROR,
		    cq_get_channel(&ch0, negative_type, qr, &qr[0]));

    int too_big_type = 2;
    TEST_ASSERT_EQUAL_INT(CQ_ERROR,
		    cq_get_channel(&ch0, too_big_type, qr, &qr[0]));

    // Providing target but accessing CQ_ADDR_GLOBAL channel.
    TEST_ASSERT_EQUAL_INT(CQ_ERROR,
		    cq_get_channel(&ch0, CQ_ADDR_GLOBAL, qr, &qr[0]));

    // Bad target
    TEST_ASSERT_EQUAL_INT(CQ_ERROR,
		    cq_get_channel(&ch0, CQ_ADDR_LOCAL, qr, NULL));

}

void test_cq_retarget_channel(channel *ch, int new_target) {
    channel ch0 = {0};

    cq_get_channel(&ch0, CQ_ADDR_LOCAL, qr, &qr[0]);

    TEST_ASSERT_EQUAL_INT(CQ_ERROR,
        cq_retarget_channel(NULL, &qr[1]));

    TEST_ASSERT_EQUAL_INT(CQ_ERROR,
        cq_retarget_channel(&ch0, NULL));

    TEST_ASSERT_EQUAL_INT(CQ_SUCCESS,
        cq_retarget_channel(&ch0, &qr[1]));
}

void test_cq_init_pulse(void) {
    pulse p = {0};
    double duration = 100.0;

    TEST_ASSERT_EQUAL_INT(CQ_SUCCESS,
                          cq_init_pulse(&p, duration));

    TEST_ASSERT_EQUAL_INT(CQ_ERROR,
                          cq_init_pulse(NULL, duration));

    double negative_duration = -100.0;
    TEST_ASSERT_EQUAL_INT(CQ_ERROR,
                          cq_init_pulse(&p, negative_duration));

    double too_long_duration = 10000000000.0;
    TEST_ASSERT_EQUAL_INT(CQ_ERROR,
                          cq_init_pulse(&p, too_long_duration));
}

void test_cq_free_pulse(void) {
    pulse p = {0};
    double duration = 100.0;
    cq_init_pulse(&p, duration);

    TEST_ASSERT_EQUAL_INT(CQ_SUCCESS,
                          cq_free_pulse(&p));
    	
    // Freeing the same pulse twice should fail
    TEST_ASSERT_EQUAL_INT(CQ_ERROR,
                          cq_free_pulse(&p));


    TEST_ASSERT_EQUAL_INT(CQ_ERROR,
                          cq_free_pulse(NULL));
}

void test_cq_play(void) {
    pulse p = {0};
    channel ch = {0};
    double duration = 100.0;

    int qreg_id = 0;
    int num_qubits = 5;
    cq_disable_analog_qreg(qr);
    cq_enable_analog_qreg(qr);

    cq_get_channel(&ch, CQ_ADDR_LOCAL, qr, &qr[0]);

    // uninitialised pulse fails to play
    TEST_ASSERT_EQUAL_INT(CQ_ERROR,
                          cq_play(&ch, &p));

    TEST_ASSERT_EQUAL_INT(CQ_ERROR,
                          cq_play(NULL, &p));

    TEST_ASSERT_EQUAL_INT(CQ_ERROR,
                          cq_play(&ch, NULL));

    // Can play initialised but "empty" pulses
    cq_init_pulse(&p, duration);
    TEST_ASSERT_EQUAL_INT(CQ_SUCCESS,
                          cq_play(&ch, &p));

    // Empty pulse will result in no change of statevector
    // If we start in zero state we will get 0 when measuring.
    int results[1];
    int num_shots = 1;
    cq_capture(&ch, &p, results, num_shots);
    TEST_ASSERT_EQUAL_INT(0, results[0]);

    // This pulse should put the statevector in |1> state.
    //double interpolation_points[3] = {0.785, 0.785, 0.785};
    //cq_interpolated_wf(p.freq, duration, interpolation_points, 3);
    cq_gaussian_wf(p.freq, duration, 2.5103, 2.0);
    TEST_ASSERT_EQUAL_INT(CQ_SUCCESS,
                          cq_play(&ch, &p));
    cq_capture(&ch, &p, results, num_shots);
    TEST_ASSERT_EQUAL_INT(1, results[0]);

}

void test_cq_capture(void) {
    int num_qubits = 5;
    int qreg_id = 0;
    cq_disable_analog_qreg(qr);
    cq_enable_analog_qreg(qr);

    channel ch = {0};
    pulse p = {0};
    int results[10];
    int num_shots = 10;
    double duration = 100.0;
    double epsilon = 0.0000001;

    // Only CQ_ADDR_LOCAL channel can capture
    cq_get_channel(&ch, CQ_ADDR_GLOBAL, qr, &qr[0]);
    TEST_ASSERT_EQUAL_INT(CQ_ERROR,
                          cq_capture(&ch, &p, results, num_shots));

    cq_get_channel(&ch, CQ_ADDR_LOCAL, qr, &qr[0]);

    // uninitialised pulse fails to capture
    TEST_ASSERT_EQUAL_INT(CQ_ERROR,
                          cq_capture(&ch, &p, results, num_shots));

    TEST_ASSERT_EQUAL_INT(CQ_ERROR,
                          cq_capture(NULL, &p, results, num_shots));

    TEST_ASSERT_EQUAL_INT(CQ_ERROR,
                          cq_capture(&ch, NULL, results, num_shots));

    TEST_ASSERT_EQUAL_INT(CQ_ERROR,
                          cq_capture(&ch, &p, NULL, num_shots));


    // Currently the pulse shape has no impact on the capture
    // so it suffices to initialise it.
    cq_init_pulse(&p, duration);
    TEST_ASSERT_EQUAL_INT(CQ_SUCCESS,
                          cq_capture(&ch, &p, results, num_shots));

    TEST_ASSERT(ch.time - (duration * num_shots) < epsilon);

    TEST_ASSERT_EQUAL_INT(0, results[0]);

    int neg_num_shots = -1;
    TEST_ASSERT_EQUAL_INT(CQ_ERROR,
                          cq_capture(&ch, &p, results, neg_num_shots));

    int too_big_num_shots = 40000;
    TEST_ASSERT_EQUAL_INT(CQ_ERROR,
                          cq_capture(&ch, &p, results, too_big_num_shots));

}

void test_cq_delay(void) {
    channel ch = {0};
    double dt = 20.0;
    double epsilon = 0.0000001;

    // uninitialised channel fails to delay
    TEST_ASSERT_EQUAL_INT(CQ_ERROR, cq_delay(&ch, dt));
 
    cq_get_channel(&ch, CQ_ADDR_GLOBAL, qr, NULL);
    TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, cq_delay(&ch, dt));
    TEST_ASSERT(ch.time - dt < epsilon);

    double negative_dt = -5.0;
    TEST_ASSERT_EQUAL_INT(CQ_ERROR, cq_delay(&ch, negative_dt));
    double too_short_dt = 5.0;
    TEST_ASSERT_EQUAL_INT(CQ_ERROR, cq_delay(&ch, too_short_dt));
    double too_long_dt = 50000000000.0;
    TEST_ASSERT_EQUAL_INT(CQ_ERROR, cq_delay(&ch, too_long_dt));
}

void test_cq_barrier(void) {
    channel ch0 = {0};
    channel ch1 = {0};
    channel ch2 = {0};
    channel ch3 = {0};
    channel ch4 = {0};

    channel *channels[5] = {&ch0, &ch1, &ch2, &ch3, &ch4};
    int num_qubits = 5;
    int qreg_id = 0;
    double epsilon = 0.0000001;

    cq_disable_analog_qreg(qr);
    cq_enable_analog_qreg(qr);

    // barrier on uninitialised channels fails
    TEST_ASSERT_EQUAL_INT(CQ_ERROR, cq_barrier(channels, num_qubits));

    for (ptrdiff_t i = 0; i < num_qubits; ++i) {
    	cq_get_channel(channels[i], CQ_ADDR_LOCAL, qr, &qr[i]);
    }
    TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, cq_barrier(channels, num_qubits));
    TEST_ASSERT(channels[0]->time - channels[1]->time < epsilon &&
                channels[1]->time - channels[2]->time < epsilon &&
                channels[2]->time - channels[3]->time < epsilon &&
                channels[3]->time - channels[4]->time < epsilon);


    int neg_num_qubits = -1;
    TEST_ASSERT_EQUAL_INT(CQ_ERROR, cq_barrier(channels, neg_num_qubits));
}

void test_cq_set_device_sample_rate(void) {
    double negative_rate = -12412.10;
    TEST_ASSERT_EQUAL_INT(CQ_ERROR, cq_set_device_sample_rate(negative_rate));

    double rate = 0.25;
    TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, cq_set_device_sample_rate(rate));
}

void test_cq_set_device_min_pulse_duration(void) {
    double negative_duration = -12412.10;
    TEST_ASSERT_EQUAL_INT(CQ_ERROR, cq_set_device_min_pulse_duration(negative_duration));

    double duration = 123.0;
    TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, cq_set_device_min_pulse_duration(duration));
}

void test_cq_set_device_max_pulse_duration(void) {
    double negative_duration = -12412.10;
    TEST_ASSERT_EQUAL_INT(CQ_ERROR, cq_set_device_max_pulse_duration(negative_duration));

    double duration = 400.0;
    TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, cq_set_device_max_pulse_duration(duration));
}

void test_cq_set_device_interaction_coeff(void) {
    double coeff = 21515.0;
    TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, cq_set_device_interaction_coeff(coeff));
    coeff = -12312.4124;
    TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, cq_set_device_interaction_coeff(coeff));
}

void test_cq_set_device_min_qubit_dist(void) {
    double dist = 0.2;
    TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, cq_set_device_min_qubit_dist(dist));
    dist = -12312.4124;
    TEST_ASSERT_EQUAL_INT(CQ_ERROR, cq_set_device_min_qubit_dist(dist));
}

void test_cq_set_device_max_num_shots(void) {
    int shots = 123123;
    TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, cq_set_device_max_num_shots(shots));

    shots = -734;
    TEST_ASSERT_EQUAL_INT(CQ_ERROR, cq_set_device_max_num_shots(shots));
}

static double dummy_coupler(double *q0, double *q1) {
    return 42.0;
};

void test_cq_set_device_coupling_func(void) {
    TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, cq_set_device_coupling_func(NULL));

    TEST_ASSERT_EQUAL_INT(CQ_SUCCESS, cq_set_device_coupling_func(&dummy_coupler));
}
