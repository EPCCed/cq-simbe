submodule (cq) device_ops
use c_device_interface
implicit none

contains
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

end submodule device_ops

