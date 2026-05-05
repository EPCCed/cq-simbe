#include <mpi.h>
#include <stdio.h>
#include <unistd.h>
#include "cq.h"
#include "datatypes.h"
#include "host_ops.h"

int main() {
  int rank;
  // MPI_Init(NULL, NULL);
  // MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  // printf("Hello from rank %d\n", rank);
  // MPI_Finalize();
  //  volatile int foo = 1;
  //  while (foo)
  //    sleep(5);

  cq_init(1);
  qubit* qr;
  alloc_qureg(&qr, 5);
  printf(
      "[MAIN]: created qreg: with NQUBITS: %zu, qregistry_idx: %d, offset: "
      "%d\n",
      qr->N, qr->registry_index, qr->offset);
  // printf("\n\nresult of free qureg: %d\n\n", free_qureg(&qr));
  cq_finalise(1);
  return 0;
}
