#include <cq.h>
#include <stdio.h>

static cq_status deterministic_kernel(const size_t count, qubit *qubits,
                                      const size_t measured, cstate *bits,
                                      struct qkern_map *registration)
{
  CQ_REGISTER_KERNEL(registration);
  if (count != 1 || measured != 1 || set_qubit(qubits[0], 0) != CQ_SUCCESS)
    return CQ_ERROR;
  /* Pull in the math-dependent gate implementation even for a static link. */
  if (unitary(&qubits[0], 0.0, 0.0, 0.0) != CQ_SUCCESS)
    return CQ_ERROR;
  return measure_qureg(qubits, count, bits);
}

int main(void)
{
  qubit *qubits = NULL;
  cstate result = -1;
  if (cq_init(0) != CQ_SUCCESS)
    return 1;
  int failed = register_qkern(deterministic_kernel) != CQ_SUCCESS;
  if (!is_device()) {
    failed |= alloc_qureg(&qubits, 1) != CQ_SUCCESS;
    if (!failed) {
      failed |= s_qrun(deterministic_kernel, qubits, 1, &result, 1) != CQ_SUCCESS;
      failed |= result != 0;
    }
    if (qubits != NULL)
      failed |= free_qureg(&qubits) != CQ_SUCCESS;
  }
  failed |= cq_finalise(0) != CQ_SUCCESS;
  if (failed)
    fprintf(stderr, "Installed cq-simbe consumer failed\n");
  return failed;
}
