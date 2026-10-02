/* Real collective regressions: rank zero cannot create its worker, or owns
   an independently initialized nondistributed QuEST environment. Both must
   return instead of stranding a peer inside the runtime capability agreement
   or QuEST communicator duplication. */
#include <stdbool.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <mpi.h>
#include "quest/include/experimental.h"
#include "src/device/mpi_runtime.h"
#include "env.h"
static int rank, borrowed, foreign_quest;
static int init_calls;
static pthread_t init_thread;

/* Observe real MPI through its standard profiling interface. */
static int observed_init(int *argc, char ***argv, int required, int *provided) {
  init_calls++;
  init_thread = pthread_self();
  return PMPI_Init_thread(argc, argv, required, provided);
}
static int observed_finalize(void) {
  if (!pthread_equal(init_thread, pthread_self())) abort();
  return PMPI_Finalize();
}
static int selective_create(pthread_t *thread, const pthread_attr_t *attr,
                            void *(*fn)(void *), void *arg) {
  return rank == 0 && !foreign_quest ? EAGAIN : pthread_create(thread, attr, fn, arg);
}
#define MPI_Init_thread observed_init
#define MPI_Finalize observed_finalize
#define pthread_create selective_create
#include "src/device/mpi_runtime.c"
#include "src/host-device/comms.c"
#include "src/host/env.c"

static cq_status init(void *arg) {
  cq_status status = cq_mpi_prepare(!isQuESTEnvInit());
  /* Reaching QuEST on one rank would hang; TIMEOUT makes that a test failure. */
  if (status == CQ_SUCCESS)
    initCustomMpiCommQuESTEnv(MPI_COMM_WORLD, -1, -1);
  return status;
}
cq_status (*control_registry[8])(void *) = {init};
#define CHECK(condition) do { if (!(condition)) { \
  fprintf(stderr, "rank %d: %s:%d: %s\n", rank, __FILE__, __LINE__, #condition); return 1; \
} } while (0)

int main(int argc, char **argv) {
  CHECK(argc == 2);
  borrowed = strcmp(argv[1], "borrowed") == 0;
  foreign_quest = strcmp(argv[1], "owned-foreign-quest") == 0;
  if (borrowed) {
    int provided;
    CHECK(observed_init(&argc, &argv, MPI_THREAD_MULTIPLE, &provided) == MPI_SUCCESS);
    CHECK(provided >= MPI_THREAD_MULTIPLE);
    CHECK(MPI_Comm_rank(MPI_COMM_WORLD, &rank) == MPI_SUCCESS);
  } else {
    /* A launcher-provided rank is needed to inject failure before MPI init.
       Support MPICH/Hydra, PMIx, Open MPI and the Intel MPI convention. */
    const char *rank_value = getenv("PMI_RANK");
    if (!rank_value) rank_value = getenv("PMIX_RANK");
    if (!rank_value) rank_value = getenv("OMPI_COMM_WORLD_RANK");
    if (!rank_value) rank_value = getenv("MPI_LOCALRANKID");
    if (!rank_value) return 77;
    rank = atoi(rank_value);
  }
  if (foreign_quest && rank == 0)
    initCustomQuESTEnv(0, 0, 0);
  CHECK(cq_init(0) == CQ_ERROR);
  CHECK(!cq_env.initialised && !cq_env.finalised);
  CHECK(!dev_ctrl.worker_started && !dev_ctrl.run_device);
  CHECK(isQuESTEnvInit() == (foreign_quest && rank == 0));
  CHECK(cq_finalise(0) == CQ_WARNING);
  CHECK(init_calls == 1);
  int finalised;
  CHECK(MPI_Finalized(&finalised) == MPI_SUCCESS);
  CHECK(finalised == !borrowed);
  if (foreign_quest && rank == 0) {
    CHECK(!getQuESTEnv().isDistributed);
    finalizeQuESTEnv();
  }
  if (borrowed) {
    CHECK(MPI_Barrier(MPI_COMM_WORLD) == MPI_SUCCESS);
    CHECK(observed_finalize() == MPI_SUCCESS);
  }
  return 0;
}
