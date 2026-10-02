#include "unity.h"
#include "test_waveforms.h"

int main(void) {
    UnityBegin("test_waveforms.c");
    RUN_TEST(test_cq_gaussian_wf);
    RUN_TEST(test_cq_gaussian_sqr_wf);
    RUN_TEST(test_cq_interpolated_wf);
    RUN_TEST(test_cq_sech_wf);
    RUN_TEST(test_cq_sin_wf);
    RUN_TEST(test_cq_cos_wf);
    RUN_TEST(test_cq_blackman_wf);
    RUN_TEST(test_cq_saw_wf);
    RUN_TEST(test_cq_custom_wf);
    RUN_TEST(test_cq_composite_wf);
    return UnityEnd();
}
