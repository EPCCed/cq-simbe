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

cq_status quantum_adiabatic_algo(const size_t NQUBITS, qubit *qr, cstate * cr, qkern_map * reg)
{
  CQ_REGISTER_KERNEL(reg);
  set_qureg(qr, 0, NQUBITS);

  int qreg_id = 0;
  HANDLE_CQ_ERROR(cq_enable_analog_qreg(qr));
  channel ch0 = {0};
  HANDLE_CQ_ERROR(cq_get_channel(&ch0, 1, qr, NULL));

  pulse pulse = {0};
  double duration = 4000.0;
  HANDLE_CQ_ERROR(cq_init_pulse(&pulse, duration));
  double data_points[3] = {1e-9, 3.0, 1e-9};
  int num_points = 3;

  HANDLE_CQ_ERROR(cq_interpolated_wf(pulse.freq, duration, data_points, num_points));

  data_points[0] = -5.0;
  data_points[1] = 0.0;
  data_points[2] = 5.0;

  HANDLE_CQ_ERROR(cq_interpolated_wf(pulse.detuning, duration, data_points, num_points));

  double positions[15] = {
        5.53855359,  1.7891413 , 0.0,
        5.48899179, -6.27296412, 0.0,
       -1.43242244, -2.26122822, 0.0,
        1.62594084, 10.99193777, 0.0,
       15.4687696 ,  2.96846731, 0.0
  };

  HANDLE_CQ_ERROR(cq_set_qubit_pos(positions, qr));
  HANDLE_CQ_ERROR(cq_play(&ch0, &pulse));

  measure_qureg(qr, NQUBITS, cr);
  HANDLE_CQ_ERROR(cq_free_pulse(&pulse));
  HANDLE_CQ_ERROR(cq_disable_analog_qreg(qr));
  return CQ_SUCCESS;
};

int main (void)
{
  const size_t NQUBITS = 5;
  const size_t NSHOTS = 10;
  const size_t NMEASURE = NQUBITS;

  cq_exec eh_maxcut;

  cq_init(0);

  cq_enable_analog_mode(ISING);
  qubit * qr = NULL;
  alloc_qureg(&qr, NQUBITS);

  cstate cr[NMEASURE * NSHOTS];

  init_creg(NMEASURE * NSHOTS, -1, cr);

  register_qkern(quantum_adiabatic_algo);

  printf("Offloading QAA circuit to the quantum device.\n");
  am_qrun(quantum_adiabatic_algo, qr, NQUBITS, cr, NMEASURE, NSHOTS, &eh_maxcut);

  printf("Hello from the host, pretend I'm doing something useful!\n");
  sleep(2);
  printf("Hello again, I'm done being 'useful' and will now wait for ");
  printf("the quantum device to return!\n");

  wait_qrun(&eh_maxcut);
  printf("Results from QAA:\n");
  report_results(cr, NMEASURE, NSHOTS);

  free_qureg(&qr);

  cq_finalise(0);

  return 0;
}
