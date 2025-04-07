#include <stddef.h>
#include <stdbool.h>
#include "resources.h"
#include "src/host-device/comms.h"
#include "quest/include/qureg.h"

quantumregistry qregistry;

void init_qregistry() {
  qregistry.num_registers = 0;
  for (size_t i = 0; i < __DEVICE_MAX_NUM_QUREGS__; ++i) {
    qregistry.available[i] = true;
  }
  return;
}

void clear_qregistry(void) {
  for (int i = 0; i < __DEVICE_MAX_NUM_QUREGS__; ++i) {
    if (!qregistry.available[i]) {
      destroyQureg(qregistry.registers[i]);
      qregistry.available[i] = true;
      --qregistry.num_registers;
    }
  }
  
  return;
}

int get_next_available_qregistry_slot() {
  int idx;
  
  // There are no available quantum register slots!
  if (qregistry.num_registers == __DEVICE_MAX_NUM_QUREGS__) return CQ_ERROR;

  for (idx = 0; idx < __DEVICE_MAX_NUM_QUREGS__; ++idx) {
    if (qregistry.available[idx]) break;
  }

  return idx;
}

cq_status device_alloc_qureg(void * par) {
  device_alloc_params * alloc_par = (device_alloc_params*) par;
  
  // find the next available free slot
  int idx = get_next_available_qregistry_slot();

  if (idx < 0) {
    // Registry is full!
    alloc_par->status = CQ_ERROR;
  } else {
    alloc_par->qregistry_idx = idx;
    qregistry.registers[alloc_par->qregistry_idx] = createQureg(alloc_par->NQUBITS);
    qregistry.available[alloc_par->qregistry_idx] = false;
    ++qregistry.num_registers;
    // TODO, check for failures on QuEST side
    alloc_par->status = CQ_SUCCESS;
  }

  return alloc_par->status;
}

cq_status device_dealloc_qureg(void * par) {
  device_alloc_params * dealloc_par = (device_alloc_params*) par;

  if (qregistry.available[dealloc_par->qregistry_idx]) {
    // register not allocated!
    dealloc_par->status = CQ_ERROR;
  } else {
    destroyQureg(qregistry.registers[dealloc_par->qregistry_idx]);
    qregistry.available[dealloc_par->qregistry_idx] = true;
    --qregistry.num_registers;
    dealloc_par->status = CQ_SUCCESS;
  }

  return dealloc_par->status;
}