/* Deterministic worker/listener failure in the real offload startup. Only
   pthread_create is injected; MPI is real and remains collectively usable. */
#include <errno.h>
#include <pthread.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cq.h"
#include "quest.h"
#include "src/host-device/comms.h"

static int rank, fail_at, creates, init_calls, finalize_calls, simulator_calls;
static int injected_failures;
static pthread_t main_thread;
static int observed_init(int *argc, char ***argv, int required, int *provided) {
  if (!pthread_equal(main_thread, pthread_self()) || required != MPI_THREAD_MULTIPLE) abort();
  ++init_calls;
  return PMPI_Init_thread(argc, argv, required, provided);
}
static int observed_finalize(void) {
  if (!pthread_equal(main_thread, pthread_self())) abort();
  ++finalize_calls;
  return PMPI_Finalize();
}
static int selective_create(pthread_t *thread, const pthread_attr_t *attr,
                            void *(*function)(void *), void *arg) {
  if (++creates == fail_at && rank == 1) {
    ++injected_failures;
    return EAGAIN;
  }
  return pthread_create(thread, attr, function, arg);
}
#define MPI_Init_thread observed_init
#define MPI_Finalize observed_finalize
#define pthread_create selective_create
#include "src/device/mpi_runtime.c"
#include "src/host-device/comms/comms_core.c"
#include "src/host-device/comms/comms_mpi.c"
#include "src/host/env.c"
#include "src/host-device/kernel_utils.c"
static cq_status start_simulator(void *unused) {
  ++simulator_calls;
  return init_quest_env();
}
static cq_status stop_simulator(void *unused) {
  finalizeQuESTEnv();
  return CQ_SUCCESS;
}
cq_status (*control_registry[8])(void *) = {start_simulator, NULL, stop_simulator};
#define CHECK(condition) do { if (!(condition)) { \
  fprintf(stderr, "rank%d line%d: %s\n", rank, __LINE__, #condition); \
  int done=0; MPI_Finalized(&done); if (!done) MPI_Abort(MPI_COMM_WORLD, 1); \
  return 1; } } while (0)
static int launcher_rank(void) {
  const char *names[] = {"PMI_RANK", "PMIX_RANK", "OMPI_COMM_WORLD_RANK", "MPI_LOCALRANKID"};
  for (size_t i=0; i<sizeof(names)/sizeof(names[0]); ++i) {
    const char *value=getenv(names[i]); if (value) return atoi(value);
  }
  return -1;
}
int main(int argc, char **argv) {
  main_thread = pthread_self();
  if (argc != 3) return 1;
  int borrowed = strcmp(argv[1], "borrowed") == 0;
  fail_at = strcmp(argv[2], "worker") == 0 ? 1 : 2;
  if (borrowed) {
    int provided;
    CHECK(observed_init(&argc, &argv, MPI_THREAD_MULTIPLE, &provided) == MPI_SUCCESS);
    CHECK(provided >= MPI_THREAD_MULTIPLE);
    CHECK(MPI_Comm_rank(MPI_COMM_WORLD, &rank) == MPI_SUCCESS);
  } else {
    rank = launcher_rank();
    if (rank < 0) return 77;
  }
  CHECK(cq_init(0) == CQ_ERROR);
  CHECK(injected_failures == (rank == 1));
  CHECK(creates == (rank == 0 ? 0 : fail_at));
  CHECK(!cq_env.initialised && !cq_env.finalised);
  CHECK(!dev_ctrl.worker_started && !dev_ctrl.run_device);
  CHECK(!dev_comm.started && !dev_comm.sync_init);
  CHECK(CQ_MPI_COMM_WORLD == MPI_COMM_NULL && CQ_MPI_SPLIT_COMM == MPI_COMM_NULL);
  CHECK(registration_comm == MPI_COMM_NULL && worker_control_comm == MPI_COMM_NULL);
  CHECK(simulator_calls == 0 && !isQuESTEnvInit());
  CHECK(cq_finalise(0) == CQ_WARNING);
  int done;
  CHECK(MPI_Finalized(&done) == MPI_SUCCESS && done == !borrowed);
  CHECK(init_calls == 1 && finalize_calls == !borrowed);
  if (borrowed) {
    /* Rejected startup must release all duplicated communicators and threads. */
    CHECK(MPI_Barrier(MPI_COMM_WORLD) == MPI_SUCCESS);
    fail_at = 0;
    CHECK(cq_init(0) == CQ_SUCCESS);
    CHECK(cq_finalise(0) == CQ_SUCCESS);
    CHECK(!isQuESTEnvInit());
    CHECK(MPI_Finalized(&done) == MPI_SUCCESS && !done);
    CHECK(MPI_Barrier(MPI_COMM_WORLD) == MPI_SUCCESS);
    CHECK(observed_finalize() == MPI_SUCCESS);
  }
  return 0;
}
