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
#include <stdbool.h>

#define CQ_ADDR_GLOBAL 0
#define CQ_ADDR_LOCAL 1

cq_status quantum_adiabatic_algo(const size_t NQUBITS, qubit *qr, const size_t NMEASURE, cstate * cr, qkern_map * reg)
{
  CQ_REGISTER_KERNEL(reg);
  HANDLE_CQ_ERROR(set_qureg(qr, 0, NQUBITS));

  /* Analog state belongs to each device process, not to the host rank. */
  static bool analog_enabled;
  if (!analog_enabled) {
    HANDLE_CQ_ERROR(cq_enable_analog_mode(ISING));
    analog_enabled = true;
  }

  HANDLE_CQ_ERROR(cq_enable_analog_qreg(qr));
  channel ch0 = {0};
  HANDLE_CQ_ERROR(cq_get_channel(&ch0, CQ_ADDR_GLOBAL, qr, NULL));

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

  HANDLE_CQ_ERROR(measure_qureg(qr, NQUBITS, cr));
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
  int failed = 0;

  if (cq_init(0) != CQ_SUCCESS) return EXIT_FAILURE;
  if (register_qkern(quantum_adiabatic_algo) != CQ_SUCCESS) {
    cq_finalise(0);
    return EXIT_FAILURE;
  }

  CQ_PROG_BEGIN()
  qubit * qr = NULL;
  cstate cr[NMEASURE * NSHOTS];
  if (alloc_qureg(&qr, NQUBITS) != CQ_SUCCESS) {
    failed = 1;
    goto host_done;
  }

  init_creg(NMEASURE * NSHOTS, -1, cr);

  printf("Offloading QAA circuit to the quantum device.\n");
  if (am_qrun(quantum_adiabatic_algo, qr, NQUBITS, cr, NMEASURE, NSHOTS, &eh_maxcut) != CQ_SUCCESS) {
    failed = 1;
    goto host_done;
  }

  printf("Hello from the host, pretend I'm doing something useful!\n");
  sleep(2);
  printf("Hello again, I'm done being 'useful' and will now wait for ");
  printf("the quantum device to return!\n");

  if (wait_qrun(&eh_maxcut) != CQ_SUCCESS || eh_maxcut.status != CQ_SUCCESS) {
    failed = 1;
  } else {
    printf("Results from QAA:\n");
    report_results(cr, NMEASURE, NSHOTS);
  }

host_done:
  if (qr && free_qureg(&qr) != CQ_SUCCESS) failed = 1;

  CQ_PROG_END()

  if (cq_finalise(0) != CQ_SUCCESS) failed = 1;

  return failed ? EXIT_FAILURE : EXIT_SUCCESS;
}
