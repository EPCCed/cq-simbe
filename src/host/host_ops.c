#include <stdlib.h>
#include "datatypes.h"
#include "host_ops.h"
#include "kernel_utils.h"
#include "src/host-device/comms.h"

// Resource Management

cq_status alloc_qureg(qubit ** qrp, size_t N) {
  cq_status status = CQ_SUCCESS;
  
  // check qr is NULL
  // If it's not, we still allocate, but issue a CQ_WARNING.
  if (*qrp != NULL)  {
    status = CQ_WARNING;
  }
  
  device_alloc_params alloc_params = {
    .NQUBITS = N,
    .qregistry_idx = 0,
    .status = CQ_ERROR
  };

  *qrp = (qubit *) malloc(sizeof(qubit) * N);
  if (*qrp == NULL) {
    /*  whoops, the malloc failed
        originally we only malloc'd on the host after receiving
        CQ_SUCCESS from the device, but it's easier to undo
        this malloc in the event the device fails to allocate a register
        than it is to undo the allocation on the device if this fails
    */
    return CQ_ERROR;
  }
  
  host_send_ctrl_op(CQ_CTRL_ALLOC, &alloc_params);
  host_wait_all_ops();

  if (alloc_params.status == CQ_SUCCESS) {
    for (size_t i = 0; i < N; ++i) {
      (*qrp)[i].registry_index = alloc_params.qregistry_idx;
      (*qrp)[i].offset = i;
      (*qrp)[i].N = N;
    }
  } else {
    // Something went wrong!
    // need to free qrp
    free(*qrp);
    *qrp = NULL;
    status = alloc_params.status;
  }

  return status;
}

cq_status free_qureg(qubit ** qrp) {
  if (*qrp == NULL) return CQ_WARNING;

  device_alloc_params dealloc_params = {
    .NQUBITS = 0,
    .qregistry_idx = (*qrp)[0].registry_index,
    .status = CQ_ERROR
  };

  host_send_ctrl_op(CQ_CTRL_DEALLOC, &dealloc_params);
  host_wait_all_ops();

  if (dealloc_params.status == CQ_SUCCESS) {
    free(*qrp);
    *qrp = NULL;
  }

  return dealloc_params.status;
}

// Executors

cq_status sm_qrun(qkern kernel, qubit * qrp, const size_t NQUBITS, 
cstate * const crp, const size_t NMEASURE, const size_t NSHOTS) {
  cq_status status = CQ_ERROR;
  char * fname = NULL;

  // proceed iff there are any shots
  // and the qreg is non-null
  // and either the creg is non-null or there will be no measurements anyway
  if (NSHOTS == 0) {
    status = CQ_SUCCESS;
  } else if (qrp != NULL && (NMEASURE == 0 || crp != NULL)) {
    status = find_qkern_name(kernel, &fname);    

    if (status == CQ_SUCCESS) {
      // safe because we wait on all ops before exiting this function
      qkern_params qk_pars[NSHOTS];   

      for (size_t shot = 0; shot < NSHOTS; ++shot) {
        qk_pars[shot].nqubits = NQUBITS;
        qk_pars[shot].fname = fname;
        qk_pars[shot].creg = crp + shot * NMEASURE;
        qk_pars[shot].qreg = qrp;
        qk_pars[shot].params = NULL;
        host_send_ctrl_op(CQ_CTRL_RUN_QKERNEL, &qk_pars[shot]);
      }
      host_wait_all_ops();
    }
  }

  return status;
}

cq_status am_qrun(qkern kernel, qubit * qrp, const size_t NQUBITS, 
cstate * const crp, const size_t NMEASURE, const size_t NSHOTS, 
cq_exec * const ehp) {
  cq_status status = CQ_ERROR;
  char * fname = NULL;

  if (NSHOTS == 0) {
    status = CQ_SUCCESS;
  } else if (qrp != NULL && (NMEASURE == 0 || crp != NULL)) {
    status = find_qkern_name(kernel, &fname);

    if (status == CQ_SUCCESS) {
      // init_exec_handle will malloc qkern_param array
      init_exec_handle(NSHOTS, ehp);

      for (size_t shot = 0; shot < NSHOTS; ++shot) {
        ehp->qk_pars[shot].nqubits = NQUBITS;
        ehp->qk_pars[shot].fname = fname;
        ehp->qk_pars[shot].qreg = qrp;
        ehp->qk_pars[shot].creg = crp + shot * NMEASURE;
        ehp->qk_pars[shot].params = NULL;
        host_send_exec(CQ_CTRL_RUN_QKERNEL, ehp, shot);
      }
    }

  }

  return status;
}

// Synchronisation

cq_status sync_qrun(cq_exec * const ehp) {
  cq_status status = CQ_ERROR;
  if (ehp->exec_init) {
    host_sync_exec(ehp);
    status = CQ_SUCCESS;
  }
  return status;  
}

cq_status wait_qrun(cq_exec * const ehp) {
  cq_status status = CQ_ERROR;
  if (ehp->exec_init) {
    size_t shots_completed = host_wait_exec(ehp);
    if (shots_completed == ehp->expected_shots) {
      status = CQ_SUCCESS;
    } else {
      status = CQ_WARNING;
    }
    finalise_exec_handle(ehp);
  }
  return status;
}