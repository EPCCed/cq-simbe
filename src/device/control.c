#include <stdio.h>
#include <stdlib.h>
#include "datatypes.h"
#include "src/host-device/comms.h"
#include "control.h"
#include "utils.h"
#include "kernel_utils.h"
#include "resources.h"
#include "quest/include/environment.h"
#include "quest/include/config.h"
#include "mpi_runtime.h"
#if QUEST_COMPILE_MPI
#include "quest/include/experimental.h"
#endif

static bool simulator_owned;

cq_status (*control_registry[8])(void *) =  {
  initialise_simulator,
  abort_current_kernel,
  finalise_simulator,
  device_alloc_qureg,
  device_dealloc_qureg,
  run_qkernel,
  run_pqkernel,
  test_control_fn
};

cq_status initialise_simulator(void * par) {
  if (simulator_owned)
    return CQ_WARNING;
  if (cq_mpi_prepare(!isQuESTEnvInit()) != CQ_SUCCESS)
    return CQ_ERROR;

  const unsigned int verbosity = *(const unsigned int *) par;
  if (verbosity > 0)
    printf("Initialising QuEST.\n");

#if QUEST_COMPILE_MPI
  initCustomMpiCommQuESTEnv(MPI_COMM_WORLD, -1, -1);
#else
  initQuESTEnv();
#endif
  if (!isQuESTEnvInit()) {
    cq_mpi_finish();
    return CQ_ERROR;
  }
  simulator_owned = true;
  if (verbosity > 0) {
    reportQuESTEnv();
    printf("Initialising quantum resource registry.\n");
  }
  init_qregistry();
  return CQ_SUCCESS;
}

cq_status abort_current_kernel(void * par) {
  return CQ_ERROR;
}

cq_status finalise_simulator(void * par) {
  if (!simulator_owned)
    return CQ_WARNING;

  const unsigned int verbosity = *(const unsigned int *) par;
  if (verbosity > 0)
    printf("Finalising QuEST\n");

  clear_qregistry();
  finalizeQuESTEnv();
  simulator_owned = false;
  cq_status status = cq_mpi_finish();
  return isQuESTEnvInit() ? CQ_ERROR : status;
}

cq_status run_qkernel(void * par) {
  cq_exec * p_exec = (cq_exec*) par;

  // find local function pointer
  qkern qk = NULL;
  cq_status status = find_qkern_pointer(p_exec->fname, &qk);

  // run it!
  if (status == CQ_SUCCESS) {
    const size_t NSHOTS = p_exec->expected_shots;
    const size_t NQUBITS = p_exec->nqubits;
    const size_t NMEASURE = p_exec->nmeasure;

    cstate * local_creg = (cstate*) malloc(NMEASURE * sizeof(cstate));

    for (size_t shot = 0; shot < NSHOTS; ++shot) {
      init_creg(NMEASURE, -1, local_creg);

      status = qk(NQUBITS, p_exec->qreg, local_creg, NULL);

      device_sync_exec(status, shot, local_creg, p_exec);
      if (status != CQ_SUCCESS) break;
    }

    free(local_creg);
  }

  return status;
}

cq_status run_pqkernel(void * par) {
  cq_exec * p_exec = (cq_exec*) par;

  // find local function pointer
  pqkern pqk = NULL;
  cq_status status = find_pqkern_pointer(p_exec->fname, &pqk);

  // run it!
  if (status == CQ_SUCCESS) {
    const size_t NSHOTS = p_exec->expected_shots;
    const size_t NQUBITS = p_exec->nqubits;
    const size_t NMEASURE = p_exec->nmeasure;

    cstate * local_creg = (cstate*) malloc(NMEASURE * sizeof(cstate));

    for (size_t shot = 0; shot < NSHOTS; ++shot) {
      init_creg(NMEASURE, -1, local_creg);

      status = pqk(NQUBITS, p_exec->qreg, local_creg, p_exec->params, NULL);

      device_sync_exec(status, shot, local_creg, p_exec);
      if (status != CQ_SUCCESS) break;
    }

    free(local_creg);
  }

 return status;
}

cq_status test_control_fn(void * par) {
  unsigned int * p_test_count = (unsigned int *) par;

  (*p_test_count)++;

  return CQ_SUCCESS;
}