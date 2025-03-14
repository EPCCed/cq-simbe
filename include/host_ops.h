#ifndef CQ_HOST_OPS_H
#define CQ_HOST_OPS_H

#include <stddef.h>
#include "datatypes.h"

// Resource management

int alloc_qubit(qubit ** qhp);

int free_qubit(qubit * qhp);

int alloc_qureg(qubit ** qrp, size_t N);

int free_qureg(qubit * qrp);

// Executors

int s_qrun(qkern kernel, qubit * qrp, const size_t NQUBITS, cstate * const crp,
  const size_t NMEASURE);

int a_qrun(qkern kernel, qubit * qrp, const size_t NQUBITS, cstate * const crp,
  const size_t NMEASURE, struct exec * const ehp);

int sm_qrun(qkern kernel, qubit * qrp, const size_t NQUBITS, 
  cstate * const crp, const size_t NMEASURE, const size_t NSHOTS);

int am_qrun(qkern kernel, qubit * qrp, const size_t NQUBITS, 
  cstate * const crp, const size_t NMEASURE, const size_t NSHOTS,
  struct exec * const ehp);

int sb_qrun(qkern kernel, qubit * qrp, const size_t NQUBITS, 
  cstate * const crp, const size_t NMEASURE, const backend_id BE);

int ab_qrun(qkern kernel, qubit * qrp, const size_t NQUBITS, 
  cstate * const crp, const size_t NMEASURE, const backend_id BE,
  struct exec * const ehp);

int smb_qrun(qkern kernel, qubit * qrp, const size_t NQUBITS, 
  cstate * const crp, const size_t NMEASURE, const size_t NSHOTS, 
  const backend_id BE);

int amb_qrun(qkern kernel, qubit * qrp, const size_t NQUBITS, 
  cstate * const crp, const size_t NMEASURE, const size_t NSHOTS, 
  const backend_id BE, struct exec * const ehp);

int sp_qrun(pqkern kernel, void * kernpar, qubit * qrp, const size_t NQUBITS,
  cstate * const crp, const size_t NMEASURE);

int ap_qrun(pqkern kernel, void * kernpar, qubit * qrp, const size_t NQUBITS,
  cstate * const crp, const size_t NMEASURE, struct exec * const ehp);

int smp_qrun(pqkern kernel, void * kernpar, qubit * qrp, const size_t NQUBITS,
  cstate * const crp, const size_t NMEASURE, const size_t NSHOTS);

int amp_qrun(pqkern kernel, void * kernpar, qubit * qrp, const size_t NQUBITS,
  cstate * const crp, const size_t NMEASURE, const size_t NSHOTS, 
  struct exec * const ehp);

int sbp_qrun(pqkern kernel, void * kernpar, qubit * qrp, const size_t NQUBITS, 
  cstate * const crp, const size_t NMEASURE, const backend_id BE);

int abp_qrun(pqkern kernel, void * kernpar, qubit * qrp, const size_t NQUBITS, 
  cstate * const crp, const size_t NMEASURE, 
  const backend_id BE, struct exec * const ehp);

int smbp_qrun(pqkern kernel, void * kernpar, qubit * qrp, const size_t NQUBITS, 
  cstate * const crp, const size_t NMEASURE, const size_t NSHOTS, 
  const backend_id BE);

int ambp_qrun(pqkern kernel, void * kernpar, qubit * qrp, const size_t NQUBITS, 
  cstate * const crp, const size_t NMEASURE, const size_t NSHOTS, 
  const backend_id BE, struct exec * const ehp);

// Synchronisation

int sync_qrun(struct exec * const ehp);

int wait_qrun(struct exec * const ehp);

int halt_qrun(struct exec * const ehp);

#endif