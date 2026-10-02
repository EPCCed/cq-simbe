#include "env.h"
#include "qft.h"
#include "cq.h"
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

int main(void) {
  int provided, nproc, rank, failed = 0;
  MPI_Comm cq_comm = MPI_COMM_NULL;
  if (MPI_Init_thread(NULL, NULL, MPI_THREAD_MULTIPLE, &provided) != MPI_SUCCESS)
    return EXIT_FAILURE;
  MPI_Comm_size(MPI_COMM_WORLD, &nproc);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  if (provided < MPI_THREAD_MULTIPLE || nproc < 4) {
    if (rank == 0) fprintf(stderr, "mpi_aqft requires MPI_THREAD_MULTIPLE and at least four ranks.\n");
    MPI_Finalize();
    return EXIT_FAILURE;
  }
  /* The first three application ranks do not participate in CQ. */
  if (MPI_Comm_split(MPI_COMM_WORLD, rank >= 3 ? 1 : MPI_UNDEFINED,
                     rank, &cq_comm) != MPI_SUCCESS) {
    MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    return EXIT_FAILURE;
  }
  const size_t NQUBITS = 10, NSHOTS = 10, NMEASURE = NQUBITS;
  cstate cr_zero[NMEASURE * NSHOTS], cr_plus[NMEASURE * NSHOTS];
  cq_exec eh_zero, eh_plus;
  qubit *qr = NULL;
  if (cq_comm != MPI_COMM_NULL) {
    if (cq_init_custom_mpi_comm(cq_comm, 1) != CQ_SUCCESS) {
      failed = 1;
    } else {
      if (register_qkern(zero_init_full_qft) != CQ_SUCCESS ||
          register_qkern(plus_init_full_qft) != CQ_SUCCESS) failed = 1;
      CQ_PROG_BEGIN()
      if (!failed && alloc_qureg(&qr, NQUBITS) == CQ_SUCCESS) {
        init_creg(NMEASURE * NSHOTS, -1, cr_zero);
        init_creg(NMEASURE * NSHOTS, -1, cr_plus);
        int zero_started = am_qrun(zero_init_full_qft, qr, NQUBITS, cr_zero,
                                   NMEASURE, NSHOTS, &eh_zero) == CQ_SUCCESS;
        int plus_started = am_qrun(plus_init_full_qft, qr, NQUBITS, cr_plus,
                                   NMEASURE, NSHOTS, &eh_plus) == CQ_SUCCESS;
        if (!zero_started || !plus_started) failed = 1;
        if (zero_started && (wait_qrun(&eh_zero) != CQ_SUCCESS || eh_zero.status != CQ_SUCCESS)) failed = 1;
        if (plus_started && (wait_qrun(&eh_plus) != CQ_SUCCESS || eh_plus.status != CQ_SUCCESS)) failed = 1;
        if (!failed) {
          printf("Results from zero-initialised QFT:\n");
          report_results(cr_zero, NMEASURE, NSHOTS);
          printf("Results from plus-initialised QFT:\n");
          report_results(cr_plus, NMEASURE, NSHOTS);
        }
        if (free_qureg(&qr) != CQ_SUCCESS) failed = 1;
      } else failed = 1;
      CQ_PROG_END()
      if (cq_finalise(0) != CQ_SUCCESS) failed = 1;
    }
    if (MPI_Comm_free(&cq_comm) != MPI_SUCCESS) failed = 1;
  }
  int any_failed = 0;
  MPI_Allreduce(&failed, &any_failed, 1, MPI_INT, MPI_MAX, MPI_COMM_WORLD);
  if (MPI_Finalize() != MPI_SUCCESS) return EXIT_FAILURE;
  return any_failed ? EXIT_FAILURE : EXIT_SUCCESS;
}
