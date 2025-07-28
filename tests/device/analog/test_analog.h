#ifndef CQ_TEST_ANALOG_H
#define CQ_TEST_ANALOG_H

void test_cq_enable_analog_mode(void);
void test_cq_samples_to_duration(void);
void test_cq_duration_to_samples(void);
void test_cq_enable_analog_qreg(void);
void test_cq_disable_analog_mode(void);
void test_cq_get_channel(void);
void test_cq_retarget_channel(void);
void test_cq_init_pulse(void);
void test_cq_free_pulse(void);
void test_cq_play(void);
void test_cq_capture(void);
void test_cq_delay(void);
void test_cq_barrier(void);

#endif
