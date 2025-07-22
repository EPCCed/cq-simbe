/**
 * Reproduces MAXCUT problem for 5 vertices
 *
 *        [-10.0, 19.7365809, 19.7365809, 5.42015853, 5.42015853],
 *        [19.7365809, -10.0, 20.67626392, 0.17675796, 0.85604541],
 *   Q =  [19.7365809, 20.67626392, -10.0, 0.85604541, 0.17675796],
 *        [5.42015853, 0.17675796, 0.85604541, -10.0, 0.32306662],
 *        [5.42015853, 0.85604541, 0.17675796, 0.32306662, -10.0],
 *
 * Solution to this problem are:
 * [1, 1, 1, 0, 0] and [1, 1, 0, 1, 0]
 *
 * Based on https://pulser.readthedocs.io/en/stable/tutorials/qubo.html
*/

#include "cq.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

void report_results(cstate const * const CR, const size_t NMEASURE,
const size_t NSHOTS) {
  printf("Reporting measurement outcomes:\n");
  for (size_t shot = 0; shot < NSHOTS; ++shot) {
    printf("Shot [%lu]: ", shot);
    for (size_t i = shot * NMEASURE; i < (shot + 1) * NMEASURE; ++i) {
      printf("%d ", CR[i]);
    }
    printf("\n");
  }

  return;
};

#define HANDLE_CQA_ERR(x)                           \
        {                                           \
            if (x == CQ_ERROR)                      \
            {                                       \
                printf(" From: %s\n", __func__);    \
                return CQ_ERROR;                    \
            }                                       \
        };

cq_status quantum_adiabatic_algo(const size_t NQUBITS, qubit *qr, cstate * cr, qkern_map * reg)
{
  CQ_REGISTER_KERNEL(reg);
  set_qureg(qr, 0, NQUBITS);

  int qreg_id = 0;
  HANDLE_CQA_ERR(cq_enable_analog_qreg(qreg_id, NQUBITS));
  channel ch0 = {0};
  HANDLE_CQA_ERR(cq_get_global_channel(&ch0, 0, qreg_id));

  pulse pulse = {0};
  double duration = 4000.0;
  HANDLE_CQA_ERR(cq_init_pulse(&pulse, duration));
  double data_points[3] = {1e-9, 3.0, 1e-9};
  int num_points = 3;

  HANDLE_CQA_ERR(cq_interpolated_wf(pulse.freq, duration, data_points, num_points));

  data_points[0] = -5.0;
  data_points[1] = 0.0;
  data_points[2] = 5.0;

  HANDLE_CQA_ERR(cq_interpolated_wf(pulse.detuning, duration, data_points, num_points));

  qpos positions[5] = {
       { 5.53855359,  1.7891413 , 0.0},
       { 5.48899179, -6.27296412, 0.0},
       {-1.43242244, -2.26122822, 0.0},
       { 1.62594084, 10.99193777, 0.0},
       {15.4687696 ,  2.96846731, 0.0}};

  HANDLE_CQA_ERR(cq_update_qreg_pos(positions, NQUBITS, qreg_id));
  HANDLE_CQA_ERR(cq_play(&ch0, &pulse));

  measure_qureg(qr, NQUBITS, cr);
  HANDLE_CQA_ERR(cq_disable_analog_qreg(qreg_id));
  return CQ_SUCCESS;
};

#undef HANDLE_CQA_ERR

int main (void)
{
  const size_t NQUBITS = 5;
  const size_t NSHOTS = 10;
  const size_t NMEASURE = NQUBITS;

  cq_exec eh_maxcut;

  cq_init(0);

  cq_enable_analog_mode(RYDBERG);
  // We will reuse the quantum buffer as the quantum
  // kernels cannot run simultaneously (for now...)
  qubit * qr = NULL;
  alloc_qureg(&qr, NQUBITS);

  cstate cr_plus[NMEASURE * NSHOTS];

  init_creg(NMEASURE * NSHOTS, -1, cr_plus);

  register_qkern(quantum_adiabatic_algo);

  printf("Offloading QAA circuit to the quantum device.\n");
  am_qrun(quantum_adiabatic_algo, qr, NQUBITS, cr, NMEASURE, NSHOTS, &eh_maxcut);

  printf("Hello from the host, pretend I'm doing something useful!\n");
  sleep(2);
  printf("Hello again, I'm done being 'useful' and will now wait for ");
  printf("the quantum device to return!\n");

  wait_qrun(&eh_maxcut);
  printf("Results from QAA:\n");
  report_results(cr_plus, NMEASURE, NSHOTS);

  free_qureg(&qr);

  cq_finalise(0);

  return 0;
}
