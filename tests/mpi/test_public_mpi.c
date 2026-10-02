/* Public API regressions. Each CTest invocation has a fresh MPI lifetime. */
#define _POSIX_C_SOURCE 200809L
#include "cq.h"
#include "quest.h"
#include "quest/include/experimental.h"
#include <mpi.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static int world_rank = -1;
static void fail(const char *condition, int line) {
  fprintf(stderr, "rank %d line %d: %s\n", world_rank, line, condition);
  int initialized = 0, finalized = 0;
  MPI_Initialized(&initialized);
  MPI_Finalized(&finalized);
  if (initialized && !finalized) MPI_Abort(MPI_COMM_WORLD, 1);
  exit(EXIT_FAILURE);
}
#define CHECK(x) do { if (!(x)) fail(#x, __LINE__); } while (0)
static void delay_ms(long ms) {
  struct timespec interval = {ms / 1000, (ms % 1000) * 1000000L};
  while (nanosleep(&interval, &interval) != 0) {}
}
static cq_status deterministic(const size_t n, qubit *qr, const size_t nm,
                               cstate *cr, qkern_map *reg) {
  CQ_REGISTER_KERNEL(reg);
  if (n != 3 || nm != 3) return CQ_ERROR;
  cq_status status = set_qureg(qr, 5, n);
  return status == CQ_SUCCESS ? measure_qureg(qr, n, cr) : status;
}
static cq_status alternate(const size_t n, qubit *qr, const size_t nm,
                           cstate *cr, qkern_map *reg) {
  CQ_REGISTER_KERNEL(reg);
  return deterministic(n, qr, nm, cr, NULL);
}
typedef struct kernel_payload { unsigned long long state; char marker[8]; } kernel_payload;
static cq_status parameterized(const size_t n, qubit *qr, const size_t nm,
                              cstate *cr, void *params, pqkern_map *reg) {
  CQ_REGISTER_KERNEL(reg);
  const kernel_payload *payload = params;
  if (!payload || strcmp(payload->marker, "copied") || n != 3 || nm != 3) return CQ_ERROR;
  cq_status status = set_qureg(qr, payload->state, n);
  return status == CQ_SUCCESS ? measure_qureg(qr, n, cr) : status;
}
static cq_status colliding_kernel(const size_t n, qubit *qr, const size_t nm,
                                  cstate *cr, qkern_map *reg) {
  if (reg) { strcpy(reg->fname, "deterministic"); return CQ_SUCCESS; }
  return CQ_ERROR;
}
static cq_status colliding_parameterized(const size_t n, qubit *qr, const size_t nm,
                                         cstate *cr, void *params, pqkern_map *reg) {
  if (reg) { strcpy(reg->fname, "parameterized"); return CQ_SUCCESS; }
  return CQ_ERROR;
}
static cq_status slow_kernel(const size_t n, qubit *qr, const size_t nm,
                            cstate *cr, qkern_map *reg) {
  CQ_REGISTER_KERNEL(reg);
  delay_ms(5);
  return deterministic(n, qr, nm, cr, NULL);
}
static cq_status distributed(const size_t n, qubit *qr, const size_t nm,
                             cstate *cr, qkern_map *reg) {
  CQ_REGISTER_KERNEL(reg);
  static int shot;
  QuESTEnv env = getQuESTEnv();
  /* Force communication despite a deliberately tiny state vector. */
  Qureg small = createCustomQureg(4, 0, 1, 0, 0);
  cq_status status = small.isDistributed && small.numNodes == env.numNodes &&
                     env.numNodes > 1 ? CQ_SUCCESS : CQ_ERROR;
  initZeroState(small);
  if (env.rank == 0 && shot == 0) {
    int entered = 1;
    MPI_Send(&entered, 1, MPI_INT, 0, 9151, MPI_COMM_WORLD);
  }
  ++shot;
  delay_ms(50);
  applyHadamard(small, 3);
  if (fabs(calcProbOfBasisState(small, 0) - 0.5) > 1e-12) status = CQ_ERROR;
  destroyQureg(small);
  if (nm) cr[0] = 1;
  return status;
}
static cq_status distributed_vote_status = CQ_EARLY_SUCCESS;
static int distributed_vote_shots;
static cq_status distributed_vote(const size_t n, qubit *qr, const size_t nm,
                                  cstate *cr, qkern_map *reg) {
  CQ_REGISTER_KERNEL(reg);
  QuESTEnv env = getQuESTEnv();
  Qureg small = createCustomQureg(4, 0, 1, 0, 0);
  cq_status status = small.isDistributed && small.numNodes == env.numNodes &&
                     env.numNodes > 1 ? CQ_SUCCESS : CQ_ERROR;
  initZeroState(small);
  applyHadamard(small, 3);
  if (fabs(calcProbOfBasisState(small, 0) - 0.5) > 1e-12) status = CQ_ERROR;
  destroyQureg(small);
  ++distributed_vote_shots;
  if (nm) cr[0] = 1;
  /* Only one worker votes to stop. Without shot consensus the peers enter
     their second distributed QuEST call and cannot finish. */
  return status == CQ_SUCCESS && env.rank == 0 ? distributed_vote_status : status;
}
static void expect_bits(const cstate *bits, size_t shots) {
  for (size_t i = 0; i < shots; ++i)
    CHECK(bits[3*i] == 1 && bits[3*i+1] == 0 && bits[3*i+2] == 1);
}
static void basic_host(void) {
  qubit *qr = NULL;
  cstate bits[12];
  init_creg(12, -1, bits);
  CHECK(alloc_qureg(&qr, 3) == CQ_SUCCESS);
  CHECK(s_qrun(deterministic, qr, 3, bits, 3) == CQ_SUCCESS);
  expect_bits(bits, 1);
  kernel_payload payload = {5, "copied"};
  CHECK(smp_qrun(parameterized, &payload, sizeof(payload), qr, 3, bits, 3, 4) == CQ_SUCCESS);
  expect_bits(bits, 4);
  CHECK(free_qureg(&qr) == CQ_SUCCESS && qr == NULL);
}
static void capacity_host(void) {
  qubit *qr = NULL;
  cq_exec handles[17], reuse;
  cstate bits[17][3], reused[3];
  CHECK(alloc_qureg(&qr, 3) == CQ_SUCCESS);
  for (size_t i = 0; i < 16; ++i) {
    init_creg(3, -1, bits[i]);
    CHECK(a_qrun(deterministic, qr, 3, bits[i], 3, &handles[i]) == CQ_SUCCESS);
  }
  CHECK(sync_qrun(&handles[0]) == CQ_SUCCESS);
  CHECK(a_qrun(deterministic, qr, 3, bits[16], 3, &handles[16]) == CQ_ERROR);
  CHECK(!handles[16].exec_init);
  size_t vacant = handles[0].id;
  CHECK(a_qrun(deterministic, qr, 3, bits[16], 3, &handles[0]) == CQ_ERROR);
  CHECK(handles[0].exec_init && handles[0].id == vacant);
  CHECK(wait_qrun(&handles[0]) == CQ_SUCCESS && handles[0].status == CQ_SUCCESS);
  expect_bits(bits[0], 1);
  CHECK(a_qrun(deterministic, qr, 3, reused, 3, &reuse) == CQ_SUCCESS);
  CHECK(reuse.id == vacant);
  for (size_t i = 1; i < 16; ++i) {
    CHECK(wait_qrun(&handles[i]) == CQ_SUCCESS && handles[i].status == CQ_SUCCESS);
    expect_bits(bits[i], 1);
  }
  CHECK(wait_qrun(&reuse) == CQ_SUCCESS && reuse.status == CQ_SUCCESS);
  expect_bits(reused, 1);
  CHECK(free_qureg(&qr) == CQ_SUCCESS);
}
static cq_status failed_kernel(const size_t n, qubit *qr, const size_t nm,
                               cstate *cr, qkern_map *reg) {
  CQ_REGISTER_KERNEL(reg);
  delay_ms(5);
  return CQ_ERROR;
}
static int launcher_rank(void) {
  const char *name[] = {"PMI_RANK", "PMIX_RANK", "OMPI_COMM_WORLD_RANK", "MPI_LOCALRANKID"};
  for (size_t i = 0; i < sizeof(name)/sizeof(name[0]); ++i) {
    const char *value = getenv(name[i]);
    if (value) return atoi(value);
  }
  return -1;
}
int main(int argc, char **argv) {
  CHECK(argc == 2);
  const char *mode = argv[1];
  int consensus = strcmp(mode, "distributed-early") == 0 || strcmp(mode, "distributed-error") == 0;
  distributed_vote_status = strcmp(mode, "distributed-error") == 0 ? CQ_ERROR : CQ_EARLY_SUCCESS;
  int owned = strncmp(mode, "owned", 5) == 0;
  int custom = strncmp(mode, "custom", 6) == 0 || strstr(mode, "split") != NULL;
  int asymmetric = strstr(mode, "asymmetric") != NULL;
  int provided = MPI_THREAD_SINGLE, size = 0, finalized = 0;
  if (!owned) {
    int required = strcmp(mode, "weak-thread") == 0 ? MPI_THREAD_SINGLE : MPI_THREAD_MULTIPLE;
    CHECK(MPI_Init_thread(&argc, &argv, required, &provided) == MPI_SUCCESS);
    CHECK(MPI_Comm_rank(MPI_COMM_WORLD, &world_rank) == MPI_SUCCESS);
    CHECK(MPI_Comm_size(MPI_COMM_WORLD, &size) == MPI_SUCCESS);
  } else world_rank = launcher_rank();
  if (owned && asymmetric && world_rank < 0) return 77;
  if (strcmp(mode, "intercomm") == 0) {
    MPI_Comm local = MPI_COMM_NULL, inter = MPI_COMM_NULL;
    CHECK(size == 4);
    int color = world_rank / 2;
    CHECK(MPI_Comm_split(MPI_COMM_WORLD, color, world_rank, &local) == MPI_SUCCESS);
    CHECK(MPI_Intercomm_create(local, 0, MPI_COMM_WORLD, color ? 0 : 2, 9152, &inter) == MPI_SUCCESS);
    CHECK(cq_init_custom_mpi_comm(inter, 0) == CQ_ERROR);
    CHECK(!cq_env.initialised && !isQuESTEnvInit());
    CHECK(cq_finalise(0) == CQ_WARNING);
    CHECK(MPI_Comm_free(&inter) == MPI_SUCCESS);
    CHECK(MPI_Comm_free(&local) == MPI_SUCCESS);
    CHECK(MPI_Barrier(MPI_COMM_WORLD) == MPI_SUCCESS);
    CHECK(MPI_Finalize() == MPI_SUCCESS);
    return 0;
  }
  if (asymmetric && world_rank == 0) {
#if QUEST_COMPILE_MPI
    if (!owned) initCustomMpiCommQuESTEnv(MPI_COMM_SELF, 0, 0);
    else initCustomQuESTEnv(0, 0, 0);
#else
    initCustomQuESTEnv(0, 0, 0);
#endif
  }
  if (strcmp(mode, "finalized") == 0) CHECK(MPI_Finalize() == MPI_SUCCESS);
  if (strcmp(mode, "weak-thread") == 0 && provided >= MPI_THREAD_MULTIPLE) {
    MPI_Finalize();
    return 77; /* This implementation promoted SINGLE; rejection is unrequestable. */
  }
  MPI_Comm comm = MPI_COMM_WORLD;
  int participating = 1;
  if (custom && !strstr(mode, "split")) CHECK(MPI_Comm_dup(MPI_COMM_WORLD, &comm) == MPI_SUCCESS);
  if (strstr(mode, "split")) {
    int color = MPI_UNDEFINED;
    if (strcmp(mode, "split-unused") == 0 && world_rank < 3) color = 0;
    if (strcmp(mode, "split-groups") == 0 && world_rank < 6) color = world_rank / 3;
    if (strcmp(mode, "split-pairs") == 0 && world_rank < 4) color = world_rank / 2;
    CHECK(MPI_Comm_split(MPI_COMM_WORLD, color, world_rank, &comm) == MPI_SUCCESS);
    participating = color != MPI_UNDEFINED;
  }
  if (strcmp(mode, "custom-null") == 0) { MPI_Comm_free(&comm); comm = MPI_COMM_NULL; }
  int reject = strstr(mode, "invalid") != NULL || strcmp(mode, "custom-null") == 0 || asymmetric ||
               strcmp(mode, "weak-thread") == 0 || strcmp(mode, "finalized") == 0;
  cq_status status = CQ_SUCCESS;
  if (participating) status = custom ? cq_init_custom_mpi_comm(comm, 0) : cq_init(0);
  if (reject) {
    CHECK(status == CQ_ERROR && !cq_env.initialised && !cq_env.finalised);
    CHECK(cq_finalise(0) == CQ_WARNING);
    CHECK(isQuESTEnvInit() == (asymmetric && world_rank == 0));
    CHECK(MPI_Finalized(&finalized) == MPI_SUCCESS);
    CHECK(finalized == (owned || strcmp(mode, "finalized") == 0));
    if (asymmetric && world_rank == 0) finalizeQuESTEnv();
    if (!finalized) CHECK(MPI_Finalize() == MPI_SUCCESS);
    return 0;
  }
  if (participating) {
    CHECK(status == CQ_SUCCESS && cq_env.initialised);
    CHECK(MPI_Comm_rank(MPI_COMM_WORLD, &world_rank) == MPI_SUCCESS);
    CHECK(MPI_Comm_size(comm, &size) == MPI_SUCCESS);
    CHECK(MPI_Query_thread(&provided) == MPI_SUCCESS && provided >= MPI_THREAD_MULTIPLE);
    CHECK(cq_init(0) == CQ_WARNING);
    int comm_rank = -1;
    CHECK(MPI_Comm_rank(comm, &comm_rank) == MPI_SUCCESS);
    CHECK(is_device() == (size > 1 && comm_rank != 0));
    CHECK(isQuESTEnvInit() == (!is_device() && size == 1 ? 1 : is_device()));
    if (isQuESTEnvInit()) CHECK(getQuESTEnv().numNodes == (size == 1 ? 1 : size-1));
    if (strcmp(mode, "delayed-registration") == 0 && world_rank == size-1) delay_ms(150);
    if (strcmp(mode, "missing-registration") == 0 || strcmp(mode, "mismatched-registration") == 0) {
      qkern kernel = deterministic;
      if (world_rank == size-1) kernel = strcmp(mode, "missing-registration") == 0 ? NULL : alternate;
      CHECK(register_qkern(kernel) == CQ_ERROR);
      if (!is_device()) {
        qubit *qr = NULL; cstate bits[3]; cq_exec exec;
        CHECK(alloc_qureg(&qr, 3) == CQ_SUCCESS);
        CHECK(a_qrun(deterministic, qr, 3, bits, 3, &exec) == CQ_ERROR);
        CHECK(!exec.exec_init);
        CHECK(free_qureg(&qr) == CQ_SUCCESS);
      }
    } else {
      CHECK(register_qkern(deterministic) == CQ_SUCCESS);
      CHECK(register_qkern(deterministic) == CQ_WARNING);
      CHECK(register_pqkern(parameterized) == CQ_SUCCESS);
      /* Distinct callbacks cannot shadow an existing remote lookup name.
         One-rank rejection must also be collectively observed. */
      CHECK(register_qkern(comm_rank == size-1 ? colliding_kernel : deterministic) == CQ_ERROR);
      CHECK(register_pqkern(comm_rank == size-1 ? colliding_parameterized : parameterized) == CQ_ERROR);
      CHECK(register_qkern(colliding_kernel) == CQ_ERROR);
      CHECK(register_pqkern(colliding_parameterized) == CQ_ERROR);
      if (strcmp(mode, "pending-finalize") == 0) CHECK(register_qkern(slow_kernel) == CQ_SUCCESS);
      if (strcmp(mode, "pending-error") == 0) CHECK(register_qkern(failed_kernel) == CQ_SUCCESS);
      if (strcmp(mode, "missing-worker") == 0) {
        /* Deliberate fault in the exported registry: exercise device lookup
           failure after a collectively successful registration. No kernel
           lookup can occur until the barrier and subsequent host submission. */
        if (world_rank == size-1) qk_reg.next_available_slot = 0;
        CHECK(MPI_Barrier(MPI_COMM_WORLD) == MPI_SUCCESS);
      }
      if (strcmp(mode, "distributed-halt") == 0) CHECK(register_qkern(distributed) == CQ_SUCCESS);
      if (consensus) CHECK(register_qkern(distributed_vote) == CQ_SUCCESS);
      if (!is_device()) {
        if (strcmp(mode, "capacity") == 0) capacity_host();
        else if (strcmp(mode, "pending-finalize") == 0 || strcmp(mode, "pending-error") == 0) {
          qubit *qr = NULL; cq_exec handles[2]; cstate bits[2][12];
          CHECK(alloc_qureg(&qr, 3) == CQ_SUCCESS);
          for (size_t i=0; i<2; ++i) {
            init_creg(12, -1, bits[i]);
            CHECK(am_qrun(strcmp(mode, "pending-error") == 0 ? failed_kernel : slow_kernel,
                          qr, 3, bits[i], 3, 4, &handles[i]) == CQ_SUCCESS);
          }
          int failed = strcmp(mode, "pending-error") == 0;
          CHECK(cq_finalise(0) == (failed ? CQ_ERROR : CQ_SUCCESS));
          for (size_t i=0; i<2; ++i) {
            CHECK(!handles[i].exec_init && handles[i].complete &&
                  handles[i].status == (failed ? CQ_ERROR : CQ_SUCCESS));
            if (!failed) expect_bits(bits[i], 4);
          }
          free(qr);
        } else if (strcmp(mode, "distributed-halt") == 0) {
          qubit *qr = NULL; cq_exec exec; cstate bits[64]; int entered = 0;
          init_creg(64, -1, bits);
          CHECK(alloc_qureg(&qr, 3) == CQ_SUCCESS);
          CHECK(am_qrun(distributed, qr, 3, bits, 1, 64, &exec) == CQ_SUCCESS);
          CHECK(MPI_Recv(&entered, 1, MPI_INT, 1, 9151, MPI_COMM_WORLD, MPI_STATUS_IGNORE) == MPI_SUCCESS);
          CHECK(entered == 1);
          CHECK(halt_qrun(&exec) == CQ_SUCCESS);
          CHECK(exec.complete && !exec.exec_init && exec.status == CQ_SUCCESS);
          CHECK(exec.completed_shots > 0 && exec.completed_shots < 64);
          for (size_t i=0; i<exec.completed_shots; ++i) CHECK(bits[i] == 1);
          CHECK(free_qureg(&qr) == CQ_SUCCESS);
        } else if (consensus) {
          qubit *qr = NULL; cq_exec exec; cstate bits[8];
          init_creg(8, -1, bits);
          CHECK(alloc_qureg(&qr, 3) == CQ_SUCCESS);
          CHECK(am_qrun(distributed_vote, qr, 3, bits, 1, 8, &exec) == CQ_SUCCESS);
          CHECK(wait_qrun(&exec) == CQ_SUCCESS);
          CHECK(exec.complete && !exec.exec_init && exec.completed_shots == 1);
          CHECK(exec.status == (distributed_vote_status == CQ_ERROR ? CQ_ERROR : CQ_SUCCESS));
          CHECK(bits[0] == 1);
          for (size_t i=1; i<8; ++i) CHECK(bits[i] == -1);
          CHECK(free_qureg(&qr) == CQ_SUCCESS);
        } else if (strcmp(mode, "missing-worker") == 0) {
          qubit *qr = NULL; cstate bits[3]; cq_exec exec;
          CHECK(alloc_qureg(&qr, 3) == CQ_SUCCESS);
          CHECK(a_qrun(deterministic, qr, 3, bits, 3, &exec) == CQ_SUCCESS);
          CHECK(wait_qrun(&exec) == CQ_SUCCESS);
          CHECK(exec.complete && !exec.exec_init && exec.status == CQ_ERROR);
          CHECK(free_qureg(&qr) == CQ_SUCCESS);
        } else basic_host();
      }
    }
    if (!((strcmp(mode, "pending-finalize") == 0 || strcmp(mode, "pending-error") == 0) && !is_device()))
      CHECK(cq_finalise(0) == (strcmp(mode, "pending-error") == 0 ? CQ_ERROR : CQ_SUCCESS));
    CHECK(cq_env.finalised && !isQuESTEnvInit());
    if (consensus) CHECK(distributed_vote_shots == (comm_rank == 0 ? 0 : 1));
    CHECK(cq_finalise(0) == CQ_WARNING && cq_init(0) == CQ_ERROR);
  }
  CHECK(MPI_Finalized(&finalized) == MPI_SUCCESS);
  CHECK(finalized == owned);
  if (!owned) {
    if (custom && participating) CHECK(MPI_Comm_free(&comm) == MPI_SUCCESS);
    CHECK(MPI_Barrier(MPI_COMM_WORLD) == MPI_SUCCESS);
    CHECK(MPI_Finalize() == MPI_SUCCESS);
  }
  return 0;
}
