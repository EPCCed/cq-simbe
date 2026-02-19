submodule (cq) device_ops
use c_device_interface
implicit none

contains
! ----------------------------- DEVICE OPERATIONS -----------------------------

  module procedure cq_register_fort_kernel !(func_name, reg) result(status)
    status = insert_to_qkern_map(func_name, reg%map)
  end procedure cq_register_fort_kernel

  module procedure cq_set_qubit !(qh, cs) result(status)
    status = set_qubit(qh%this, cs)
  end procedure cq_set_qubit

  module procedure cq_set_qureg !(qr, STATE_IDX, NQUBITS) result(status)
    status = set_qureg(qr%this, STATE_IDX, NQUBITS)
  end procedure cq_set_qureg

  module procedure cq_set_qureg_cstate !(qr, CR, N) result(status)
    status = set_qureg_cstate(qr%this, CR, N)
  end procedure cq_set_qureg_cstate

  module procedure cq_qabort !(STATUS) result(result)
    result = qabort(STATUS)
  end procedure cq_qabort

  module procedure cq_dmeasure_qubit !(qbp, csp) result(status)
    status = dmeasure_qubit(qbp%this, c_loc(csp))
  end procedure cq_dmeasure_qubit

  module procedure cq_dmeasure_qureg !(qr, NQUBITS, cr) result(status)
    status = dmeasure_qureg(qr%this, NQUBITS, cr)
  end procedure cq_dmeasure_qureg

  module procedure cq_dmeasure !(qr, NQUBITS, TARGETS, NTARGETS, cr) result(status)
    status = dmeasure(qr%this, NQUBITS, TARGETS, NTARGETS, cr)
  end procedure cq_dmeasure

  module procedure cq_measure_qubit !(qbp, csp) result(status)
    status = measure_qubit(qbp%this, c_loc(csp))
  end procedure cq_measure_qubit

  module procedure cq_measure_qureg !(qr, NQUBITS, cr) result(status)
    status = measure_qureg(qr%this, NQUBITS, cr)
  end procedure cq_measure_qureg

  module procedure cq_measure !(qr, NQUBITS, TARGETS, NTARGETS, cr) result(status)
    status = measure(qr%this, NQUBITS, TARGETS, NTARGETS, cr)
  end procedure cq_measure

end submodule device_ops

