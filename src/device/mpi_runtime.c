#include <stdbool.h>
#include <limits.h>
#include "mpi_runtime.h"
#include "quest/include/config.h"

#if QUEST_COMPILE_MPI
#if !QUEST_COMPILE_SUBCOMM
#error "MPI-enabled QuEST must be built with QUEST_ENABLE_SUBCOMM=ON"
#endif
#include <mpi.h>

static bool preflight_done;
static bool borrowed_mpi;
static bool owned_mpi;
static MPI_Comm shot_comm = MPI_COMM_NULL;

/* All ranks decide before any rank can enter QuEST's collective initialiser. */
static cq_status agree_capability(int local_ready) {
  int size = 0, all_ready = 0;
  if (MPI_Comm_size(MPI_COMM_WORLD, &size) != MPI_SUCCESS)
    local_ready = 0;
  local_ready = local_ready && size > 0 && (size & (size - 1)) == 0;
  if (MPI_Allreduce(&local_ready, &all_ready, 1, MPI_INT, MPI_MIN,
                    MPI_COMM_WORLD) != MPI_SUCCESS)
    return CQ_ERROR;
  return all_ready ? CQ_SUCCESS : CQ_ERROR;
}

cq_status cq_mpi_preflight(int local_ready) {
  int finalised = 0, initialised = 0, provided = MPI_THREAD_SINGLE;
  preflight_done = false;
  if (MPI_Finalized(&finalised) != MPI_SUCCESS || finalised ||
      MPI_Initialized(&initialised) != MPI_SUCCESS)
    return CQ_ERROR;
  borrowed_mpi = initialised;
  if (borrowed_mpi) {
    /* This must run on the caller, before creating the worker: even querying
       threading from the worker is illegal for caller-owned FUNNELED MPI. */
    if (MPI_Query_thread(&provided) != MPI_SUCCESS)
      local_ready = 0;
    local_ready = local_ready && provided >= MPI_THREAD_MULTIPLE;
    if (agree_capability(local_ready) != CQ_SUCCESS)
      return CQ_ERROR;
  }
  /* With uninitialised MPI, defer local rejection until prepare(): peers
     must all enter MPI_Init_thread and agree before any enters QuEST. The
     worker passes its current QuEST readiness to that collective check. */
  preflight_done = true;
  return CQ_SUCCESS;
}

void cq_mpi_cancel_preflight(void) {
  preflight_done = false;
  borrowed_mpi = false;
}

cq_status cq_mpi_prepare(int local_ready) {
  /* Direct device tests call initialise_simulator on their current thread. */
  if (!preflight_done && cq_mpi_preflight(local_ready) != CQ_SUCCESS)
    return CQ_ERROR;
  if (borrowed_mpi) {
    if (agree_capability(local_ready) != CQ_SUCCESS) return CQ_ERROR;
    return MPI_Comm_dup(MPI_COMM_WORLD, &shot_comm) == MPI_SUCCESS ? CQ_SUCCESS : CQ_ERROR;
  }

  int provided = MPI_THREAD_SINGLE;
  if (MPI_Init_thread(NULL, NULL, MPI_THREAD_MULTIPLE, &provided) != MPI_SUCCESS) {
    cq_mpi_cancel_preflight();
    return CQ_ERROR;
  }
  owned_mpi = true;
  if (agree_capability(local_ready && provided >= MPI_THREAD_MULTIPLE) != CQ_SUCCESS) {
    /* Init and Finalize must occur on this same worker, even on rejection. */
    cq_mpi_finish();
    return CQ_ERROR;
  }
  if (MPI_Comm_dup(MPI_COMM_WORLD, &shot_comm) != MPI_SUCCESS) {
    cq_mpi_finish();
    return CQ_ERROR;
  }
  return CQ_SUCCESS;
}

cq_status cq_mpi_finish(void) {
  cq_status status = CQ_SUCCESS;
  if (shot_comm != MPI_COMM_NULL && MPI_Comm_free(&shot_comm) != MPI_SUCCESS)
    status = CQ_ERROR;
  if (owned_mpi) {
    if (MPI_Finalize() != MPI_SUCCESS)
      status = CQ_ERROR;
    owned_mpi = false;
  }
  cq_mpi_cancel_preflight();
  return status;
}
cq_status cq_mpi_agree_status(cq_status status, bool halt) {
  int local = status != CQ_SUCCESS && status != CQ_EARLY_SUCCESS ? 2 :
              (halt || status == CQ_EARLY_SUCCESS ? 1 : 0);
  int agreed = local;
  if (shot_comm != MPI_COMM_NULL)
    MPI_Allreduce(&local, &agreed, 1, MPI_INT, MPI_MAX, shot_comm);
  if (agreed == 2) {
    int error = local == 2 ? (int)status : INT_MAX, result = error;
    if (shot_comm != MPI_COMM_NULL)
      MPI_Allreduce(&error, &result, 1, MPI_INT, MPI_MIN, shot_comm);
    return result;
  }
  return agreed == 1 ? CQ_EARLY_SUCCESS : CQ_SUCCESS;
}

#else
cq_status cq_mpi_preflight(int local_ready) {
  return local_ready ? CQ_SUCCESS : CQ_ERROR;
}
void cq_mpi_cancel_preflight(void) {}
cq_status cq_mpi_prepare(int local_ready) {
  return cq_mpi_preflight(local_ready);
}
cq_status cq_mpi_finish(void) { return CQ_SUCCESS; }
cq_status cq_mpi_agree_status(cq_status status, bool halt) {
  return halt && status == CQ_SUCCESS ? CQ_EARLY_SUCCESS : status;
}
#endif
