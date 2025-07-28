#include "unity.h"
#include "cq.h"
#include "test_analog.h"

int main(void) {
    UnityBegin("test_analog.c");

    cq_init(0);

    RUN_TEST(test_cq_enable_analog_mode);
    RUN_TEST(test_cq_samples_to_duration);
    RUN_TEST(test_cq_duration_to_samples);
    RUN_TEST(test_cq_enable_analog_qreg);
    RUN_TEST(test_cq_disable_analog_mode);
    //RUN_TEST(test_cq_get_global_channel);
    //RUN_TEST(test_cq_get_local_channel);
    RUN_TEST(test_cq_get_channel);
    //RUN_TEST(test_cq_update_qreg_pos);
    RUN_TEST(test_cq_init_pulse);
    RUN_TEST(test_cq_free_pulse);
    RUN_TEST(test_cq_play);
    RUN_TEST(test_cq_capture);
    RUN_TEST(test_cq_delay);
    RUN_TEST(test_cq_barrier);

    cq_finalise(0);

    return UnityEnd();
}
