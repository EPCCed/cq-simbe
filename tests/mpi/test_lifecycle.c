#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cq.h"
#include "quest/include/config.h"
#include "quest/include/environment.h"
#if QUEST_COMPILE_MPI
#include <mpi.h>
#include "quest/include/experimental.h"
#endif

#define CHECK(condition) do { if (!(condition)) { \
  fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #condition); return 1; \
} } while (0)

static cq_status deterministic_kernel(const size_t n, qubit *qr, cstate *cr,
                                     qkern_map *reg) {
  CQ_REGISTER_KERNEL(reg);
  cq_status status = set_qureg(qr, 5, n);
  return status == CQ_SUCCESS ? measure_qureg(qr, n, cr) : status;
}

int main(int argc, char **argv) {
  CHECK(argc == 2);
  const char *mode = argv[1];
#if QUEST_COMPILE_MPI
  int provided = MPI_THREAD_SINGLE, finalised = 0;
  const int borrowed = strncmp(mode, "owned", 5) != 0;
  if (borrowed) {
    int required = strcmp(mode, "insufficient") == 0 ? MPI_THREAD_SINGLE : MPI_THREAD_MULTIPLE;
    CHECK(MPI_Init_thread(&argc, &argv, required, &provided) == MPI_SUCCESS);
    if (strcmp(mode, "insufficient") != 0) CHECK(provided >= MPI_THREAD_MULTIPLE);
  }
  if (strcmp(mode, "finalized") == 0) CHECK(MPI_Finalize() == MPI_SUCCESS);
  if (strcmp(mode, "foreign-quest") == 0)
    initCustomMpiCommQuESTEnv(MPI_COMM_WORLD, -1, -1);
  if (strcmp(mode, "finalized") == 0 || strstr(mode, "nonpower") != NULL ||
      strcmp(mode, "foreign-quest") == 0 ||
      (strcmp(mode, "insufficient") == 0 && provided < MPI_THREAD_MULTIPLE)) {
    CHECK(cq_init(0) == CQ_ERROR);
    CHECK(!cq_env.initialised);
    CHECK(cq_finalise(0) == CQ_WARNING);
    CHECK(!cq_env.finalised);
    CHECK(MPI_Finalized(&finalised) == MPI_SUCCESS);
    CHECK(finalised == (!borrowed || strcmp(mode, "finalized") == 0));
    if (strcmp(mode, "foreign-quest") == 0) {
      CHECK(isQuESTEnvInit());
      finalizeQuESTEnv();
    } else CHECK(!isQuESTEnvInit());
    if (!finalised) CHECK(MPI_Finalize() == MPI_SUCCESS);
    return 0;
  }
#else
  if (strcmp(mode, "foreign-quest") == 0) {
    initQuESTEnv();
    CHECK(cq_init(0) == CQ_ERROR);
    CHECK(!cq_env.initialised);
    CHECK(cq_finalise(0) == CQ_WARNING);
    CHECK(isQuESTEnvInit());
    finalizeQuESTEnv();
    return 0;
  }
#endif
  CHECK(cq_init(0) == CQ_SUCCESS);
  CHECK(cq_env.initialised);
  CHECK(cq_init(0) == CQ_WARNING);
  CHECK(isQuESTEnvInit());
#if QUEST_COMPILE_MPI
  int world_size;
  CHECK(MPI_Comm_size(MPI_COMM_WORLD, &world_size) == MPI_SUCCESS);
  CHECK(getQuESTEnv().numNodes == world_size);
  CHECK(getQuESTEnv().isMpiUserOwned);
  CHECK(MPI_Query_thread(&provided) == MPI_SUCCESS);
  CHECK(provided >= MPI_THREAD_MULTIPLE);
#endif
  CHECK(register_qkern(deterministic_kernel) == CQ_SUCCESS);
  qubit *qr = NULL;
  cstate cr[3] = {-1, -1, -1};
  CHECK(alloc_qureg(&qr, 3) == CQ_SUCCESS);
  CHECK(s_qrun(deterministic_kernel, qr, 3, cr, 3) == CQ_SUCCESS);
  CHECK(cr[0] == 1 && cr[1] == 0 && cr[2] == 1);
  /* Leave the register live to exercise cleanup before QuEST/MPI teardown. */
  CHECK(cq_finalise(0) == CQ_SUCCESS);
  free(qr);
  CHECK(!isQuESTEnvInit());
  CHECK(cq_finalise(0) == CQ_WARNING);
  CHECK(cq_init(0) == CQ_ERROR);
#if QUEST_COMPILE_MPI
  CHECK(MPI_Finalized(&finalised) == MPI_SUCCESS);
  CHECK(finalised == !borrowed);
  if (borrowed) {
    CHECK(MPI_Barrier(MPI_COMM_WORLD) == MPI_SUCCESS);
    CHECK(MPI_Finalize() == MPI_SUCCESS);
  }
#endif
  return 0;
}
