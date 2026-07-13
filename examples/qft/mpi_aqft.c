#include "env.h"
#include "qft.h"

#include "cq.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <mpi.h>

int main(void) {
  int provided;

  MPI_Comm cq_comm;
  int nproc;
  int rank;
  MPI_Init_thread(NULL, NULL, MPI_THREAD_MULTIPLE, &provided);
  MPI_Comm_size(MPI_COMM_WORLD, &nproc);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  const int split_cond = rank >= 3;
  MPI_Comm_split(MPI_COMM_WORLD, split_cond, rank, &cq_comm);
  const size_t NQUBITS = 10;
  const size_t NSHOTS = 10;
  const size_t NMEASURE = NQUBITS;

  cstate cr_zero[NMEASURE * NSHOTS];
  cstate cr_plus[NMEASURE * NSHOTS];
  cq_exec eh_zero, eh_plus;

  qubit * qr = NULL;

  if (split_cond) {
    cq_init_custom_mpi_comm(cq_comm, 1);

    register_qkern(zero_init_full_qft);
    register_qkern(plus_init_full_qft);

    CQ_PROG_BEGIN()

    // We will reuse the quantum buffer as the quantum
    // kernels cannot run simultaneously (for now...)
    alloc_qureg(&qr, NQUBITS);

    init_creg(NMEASURE * NSHOTS, -1, cr_zero);
    init_creg(NMEASURE * NSHOTS, -1, cr_plus);

    printf("Offloading both QFT circuits to the quantum device.\n");
    am_qrun(zero_init_full_qft, qr, NQUBITS, cr_zero, NMEASURE, NSHOTS,
            &eh_zero);
    am_qrun(plus_init_full_qft, qr, NQUBITS, cr_plus, NMEASURE, NSHOTS,
            &eh_plus);

    CQ_PROG_END()
  }

  if (rank < 3) {
    printf("Hello from the host, pretend I'm doing something useful!\n");
    sleep(2);
    printf("Hello again, I'm done being 'useful' and will now wait for ");
    printf("the quantum device to return!\n");
  }

  if (split_cond) {
    CQ_PROG_BEGIN()

    wait_qrun(&eh_zero);
    wait_qrun(&eh_plus);

    free_qureg(&qr);

    printf("Results from zero-initialised QFT:\n");
    report_results(cr_zero, NMEASURE, NSHOTS);

    printf("Results from plus-initialised QFT:\n");
    report_results(cr_plus, NMEASURE, NSHOTS);

    CQ_PROG_END()
    cq_finalise(0);
  }

  MPI_Finalize();

  return 0;
}
