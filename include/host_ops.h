#ifndef CQ_HOST_OPS_H
#define CQ_HOST_OPS_H

#include <stddef.h>
#include "datatypes.h"

// Resource management

cq_status alloc_qubit(qubit ** qhp);

cq_status free_qubit(qubit ** qhp);

cq_status alloc_qureg(qubit ** qrp, size_t N);

cq_status free_qureg(qubit ** qrp);

// Executors

cq_status s_qrun(qkern kernel, qubit * qrp, const size_t NQUBITS, cstate * const crp,
  const size_t NMEASURE);

cq_status a_qrun(qkern kernel, qubit * qrp, const size_t NQUBITS, cstate * const crp,
  const size_t NMEASURE, cq_exec * const ehp);

cq_status sm_qrun(qkern kernel, qubit * qrp, const size_t NQUBITS, 
  cstate * const crp, const size_t NMEASURE, const size_t NSHOTS);

cq_status am_qrun(qkern kernel, qubit * qrp, const size_t NQUBITS, 
  cstate * const crp, const size_t NMEASURE, const size_t NSHOTS,
  cq_exec * const ehp);

cq_status sb_qrun(qkern kernel, qubit * qrp, const size_t NQUBITS, 
  cstate * const crp, const size_t NMEASURE, const backend_id BE);

cq_status ab_qrun(qkern kernel, qubit * qrp, const size_t NQUBITS, 
  cstate * const crp, const size_t NMEASURE, const backend_id BE,
  cq_exec * const ehp);

cq_status smb_qrun(qkern kernel, qubit * qrp, const size_t NQUBITS, 
  cstate * const crp, const size_t NMEASURE, const size_t NSHOTS, 
  const backend_id BE);

cq_status amb_qrun(qkern kernel, qubit * qrp, const size_t NQUBITS, 
  cstate * const crp, const size_t NMEASURE, const size_t NSHOTS, 
  const backend_id BE, cq_exec * const ehp);

cq_status sp_qrun(pqkern kernel, void * kernpar, qubit * qrp, const size_t NQUBITS,
  cstate * const crp, const size_t NMEASURE);

cq_status ap_qrun(pqkern kernel, void * kernpar, qubit * qrp, const size_t NQUBITS,
  cstate * const crp, const size_t NMEASURE, cq_exec * const ehp);

cq_status smp_qrun(pqkern kernel, void * kernpar, qubit * qrp, const size_t NQUBITS,
  cstate * const crp, const size_t NMEASURE, const size_t NSHOTS);

cq_status amp_qrun(pqkern kernel, void * kernpar, qubit * qrp, const size_t NQUBITS,
  cstate * const crp, const size_t NMEASURE, const size_t NSHOTS, 
  cq_exec * const ehp);

cq_status sbp_qrun(pqkern kernel, void * kernpar, qubit * qrp, const size_t NQUBITS, 
  cstate * const crp, const size_t NMEASURE, const backend_id BE);

cq_status abp_qrun(pqkern kernel, void * kernpar, qubit * qrp, const size_t NQUBITS, 
  cstate * const crp, const size_t NMEASURE, 
  const backend_id BE, cq_exec * const ehp);

cq_status smbp_qrun(pqkern kernel, void * kernpar, qubit * qrp, const size_t NQUBITS, 
  cstate * const crp, const size_t NMEASURE, const size_t NSHOTS, 
  const backend_id BE);

cq_status ambp_qrun(pqkern kernel, void * kernpar, qubit * qrp, const size_t NQUBITS, 
  cstate * const crp, const size_t NMEASURE, const size_t NSHOTS, 
  const backend_id BE, cq_exec * const ehp);

// Synchronisation

cq_status sync_qrun(cq_exec * const ehp);

cq_status wait_qrun(cq_exec * const ehp);

cq_status halt_qrun(cq_exec * const ehp);

#endif