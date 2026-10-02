/* MPI capability failures are not reliably requestable from an MPI runtime.
   Compile the real helper against a deterministic shim, without interposing
   any functions in the production library or the installed MPI library. */
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <mpi.h>
#include "src/device/mpi_runtime.h"

static pthread_t caller, init_thread;
static int initialised, finalised, provided = MPI_THREAD_MULTIPLE;
static int query_count, init_count, finalize_count, wrong_thread;
static int peer_ready = 1, world_size = 2, init_error, late_peer, reductions, local_unready;
#define CHECK(condition) do { if (!(condition)) { \
  fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #condition); return 1; \
} } while (0)

static int shim_finalized(int *value) { *value = finalised; return MPI_SUCCESS; }
static int shim_initialized(int *value) { *value = initialised; return MPI_SUCCESS; }
static int shim_query(int *value) {
  query_count++;
  if (!pthread_equal(pthread_self(), caller)) wrong_thread++;
  *value = provided;
  return MPI_SUCCESS;
}
static int shim_init(int *argc, char ***argv, int required, int *value) {
  init_count++;
  if (required != MPI_THREAD_MULTIPLE || pthread_equal(pthread_self(), caller))
    wrong_thread++;
  if (init_error) return MPI_ERR_OTHER;
  init_thread = pthread_self();
  initialised = 1;
  *value = provided;
  return MPI_SUCCESS;
}
static int shim_finalize(void) {
  finalize_count++;
  if (!pthread_equal(pthread_self(), init_thread)) wrong_thread++;
  finalised = 1;
  return MPI_SUCCESS;
}
static int shim_size(MPI_Comm comm, int *size) { *size = world_size; return MPI_SUCCESS; }
static int shim_reduce(const void *in, void *out, int count, MPI_Datatype datatype,
                       MPI_Op op, MPI_Comm comm) {
  reductions++;
  *(int *)out = *(const int *)in && peer_ready && !(late_peer && reductions > 1);
  return MPI_SUCCESS;
}
#define MPI_Finalized shim_finalized
#define MPI_Initialized shim_initialized
#define MPI_Query_thread shim_query
#define MPI_Init_thread shim_init
#define MPI_Finalize shim_finalize
#define MPI_Comm_size shim_size
#define MPI_Allreduce shim_reduce
#include "src/device/mpi_runtime.c"

/* Use the production host and worker so rejection also exercises queue
   status propagation, joining the worker, and no-worker finalisation. */
#include "src/host-device/comms.c"
#include "src/host/env.c"
int isQuESTEnvInit(void) { return local_unready; }
static cq_status init(void *arg) { return cq_mpi_prepare(!isQuESTEnvInit()); }
static cq_status finish(void *arg) { return cq_mpi_finish(); }
cq_status (*control_registry[8])(void *) = {init, NULL, finish};

int main(int argc, char **argv) {
  CHECK(argc == 2);
  caller = pthread_self();
  int borrowed = strncmp(argv[1], "borrowed", 8) == 0;
  initialised = borrowed;
  if (strstr(argv[1], "low")) provided = MPI_THREAD_FUNNELED;
  if (strstr(argv[1], "peer")) { provided = MPI_THREAD_MULTIPLE; peer_ready = 0; }
  if (strstr(argv[1], "late")) { peer_ready = 1; late_peer = 1; }
  if (strstr(argv[1], "local-unready")) local_unready = 1;
  if (strstr(argv[1], "nonpower")) world_size = 3;
  if (strcmp(argv[1], "finalized") == 0) finalised = 1;
  if (strcmp(argv[1], "init-error") == 0) init_error = 1;
  int rejected = provided < MPI_THREAD_MULTIPLE || !peer_ready || world_size == 3 || init_error || late_peer || local_unready;
  cq_status status = cq_init(0);
  CHECK(status == (rejected || finalised ? CQ_ERROR : CQ_SUCCESS));
  if (status == CQ_ERROR) {
    CHECK(!cq_env.initialised);
    CHECK(!dev_ctrl.worker_started);
    CHECK(cq_finalise(0) == CQ_WARNING);
  } else {
    CHECK(cq_finalise(0) == CQ_SUCCESS);
  }
  CHECK(!dev_ctrl.worker_started);
  if (strcmp(argv[1], "finalized") == 0 || (borrowed && rejected)) {
    CHECK(init_count == 0 && finalize_count == 0 && wrong_thread == 0);
    CHECK(query_count == borrowed);
    return 0;
  }
  CHECK(wrong_thread == 0);
  CHECK(query_count == borrowed);
  CHECK(init_count == !borrowed);
  CHECK(finalize_count == (!borrowed && !init_error));
  CHECK(finalised == (!borrowed && !init_error));
  return 0;
}
