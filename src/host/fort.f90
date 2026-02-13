submodule (cq) host_ops

use c_host_interface
use c_device_interface
implicit none

contains

! ------------------------------ HOST OPERATIONS ------------------------------

  module procedure fcq_init !(VERBOSITY) result(status)
    status = cq_init(VERBOSITY)
  end procedure fcq_init

  module procedure fcq_finalise !(VERBOSITY) result(status)
    status = cq_finalise(VERBOSITY)
  end procedure fcq_finalise

  module procedure cq_alloc_qureg !(qrp, N) result(status)
    status = alloc_qureg(qrp%this, N)
  end procedure cq_alloc_qureg

  module procedure cq_free_qureg !(qrp) result(status)
    status = free_qureg(qrp%this)
  end procedure cq_free_qureg

  module procedure cq_init_creg !(LENGTH, INIT_VAL, cr) result(status)
    call init_creg(LENGTH, INIT_VAL, cr)
  end procedure cq_init_creg

  module procedure cq_register_qkern !(kernel)
    status = register_qkern(kernel)
  end procedure cq_register_qkern

  module procedure cq_sm_qrun !(kernel, qrp, NQUBITS, crp, NMEASURE, NSHOTS) bind(C) result(status)
    status = sm_qrun(kernel, qrp%this, NQUBITS, crp, NMEASURE, NSHOTS)
  end procedure

! ----------------------------- DEVICE OPERATIONS -----------------------------

  module procedure cq_register_fort_kernel !(func_name, reg) result(status)
    status = insert_to_qkern_map(func_name, reg%map)
  end procedure cq_register_fort_kernel

  module procedure cq_set_qureg !(qr, STATE_IDX, NQUBITS) result(status)
    status = set_qureg(qr%this, STATE_IDX, NQUBITS)
  end procedure cq_set_qureg

  module procedure cq_measure_qureg !(qr, NQUBITS, cr) result(status)
    status = measure_qureg(qr%this, NQUBITS, cr)
  end procedure cq_measure_qureg

! --------------------------------- QASM GATES --------------------------------

  module procedure cq_hadamard !(qh, qubit_idx) result(status)
    type(c_ptr) :: curr_ptr
    integer(c_intptr_t) :: offset
    type(c_ptr) :: new_ptr
    type(qubit_size) :: tmp
    curr_ptr = qh%this
    offset = transfer(curr_ptr, offset) + (c_sizeof(tmp) * qubit_idx)
    new_ptr = transfer(offset, new_ptr)
    status = hadamard(new_ptr)
  end procedure cq_hadamard

end submodule host_ops


