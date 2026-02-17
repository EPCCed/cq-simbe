submodule (cq) qasm_gates
use c_device_interface
implicit none

contains
! --------------------------------- QASM GATES --------------------------------

  function get_qubit_at(qh, qubit_idx) result(new_ptr)
    implicit none
    type(qubit), value :: qh
    integer(c_intptr_t), value :: qubit_idx
    type(c_ptr) :: curr_ptr
    integer(c_intptr_t) :: offset
    type(c_ptr) :: new_ptr
    type(qubit_size) :: tmp

    ! validate representation of c_intptr_t and c_ptr
    integer(c_intptr_t) :: unused_i
    type(c_ptr) :: unused_p
    integer, parameter :: pointer_sized = &
    merge(c_intptr_t, -1, storage_size(unused_i) == storage_size(unused_p))
    integer(pointer_sized) :: ip
    ! end

    curr_ptr = qh%this

    offset = transfer(curr_ptr, offset) + (c_sizeof(tmp) * qubit_idx)
    new_ptr = transfer(offset, new_ptr)
  end function get_qubit_at

  module procedure cq_unitary !(qh, qubit_idx, THETA, PHI, LAMBDA) result(status)
    status = unitary(get_qubit_at(qh, qubit_idx), THETA, PHI, LAMBDA)
  end procedure cq_unitary

  module procedure cq_gphase !(qh, qubit_idx, THETA) result(status)
    status = gphase(get_qubit_at(qh, qubit_idx), THETA)
  end procedure cq_gphase

  module procedure cq_paulix !(qh, qubit_idx) result(status) 
    status = paulix(get_qubit_at(qh, qubit_idx))
  end procedure cq_paulix

  module procedure cq_pauliy !(qh, qubit_idx) result(status) 
    status = pauliy(get_qubit_at(qh, qubit_idx))
  end procedure cq_pauliy

  module procedure cq_pauliz !(qh, qubit_idx) result(status) 
    status = pauliz(get_qubit_at(qh, qubit_idx))
  end procedure cq_pauliz

  module procedure cq_hadamard !(qh, qubit_idx) result(status)
    status = hadamard(get_qubit_at(qh, qubit_idx))
  end procedure cq_hadamard

  module procedure cq_sqrtz !(qh, qubit_idx) result(status) 
    status = sqrtz(get_qubit_at(qh, qubit_idx))
  end procedure cq_sqrtz

  module procedure cq_sqrtzhc !(qh, qubit_idx) result(status) 
    status = sqrtzhc(get_qubit_at(qh, qubit_idx))
  end procedure cq_sqrtzhc

  module procedure cq_sqrts !(qh, qubit_idx) result(status) 
    status = sqrts(get_qubit_at(qh, qubit_idx))
  end procedure cq_sqrts

  module procedure cq_sqrtshc !(qh, qubit_idx) result(status) 
    status = sqrtshc(get_qubit_at(qh, qubit_idx))
  end procedure cq_sqrtshc

  module procedure cq_sqrtx !(qh, qubit_idx) result(status) 
    status = sqrtx(get_qubit_at(qh, qubit_idx))
  end procedure cq_sqrtx

  module procedure cq_rotx !(qh, qubit_idx, THETA) result(status) 
    status = rotx(get_qubit_at(qh, qubit_idx), THETA)
  end procedure cq_rotx

  module procedure cq_roty !(qh, qubit_idx, THETA) result(status) 
    status = roty(get_qubit_at(qh, qubit_idx), THETA)
  end procedure cq_roty

  module procedure cq_rotz !(qh, qubit_idx, THETA) result(status) 
    status = rotz(get_qubit_at(qh, qubit_idx), THETA)
  end procedure cq_rotz
 
  module procedure cq_cpaulix !(qr, ctrl, qtarget) result(status) 
    status = cpaulix(get_qubit_at(qr, ctrl), get_qubit_at(qr, qtarget))
  end procedure cq_cpaulix

  module procedure cq_cpauliy !(qr, ctrl, qtarget) result(status) 
    status = cpauliy(get_qubit_at(qr, ctrl), get_qubit_at(qr, qtarget))
  end procedure cq_cpauliy

  module procedure cq_cpauliz !(qr, ctrl, qtarget) result(status) 
    status = cpauliz(get_qubit_at(qr, ctrl), get_qubit_at(qr, qtarget))
  end procedure cq_cpauliz

  module procedure cq_cphase !(qr, ctrl, qtarget, THETA) result(status) 
    status = cphase(get_qubit_at(qr, ctrl), get_qubit_at(qr, qtarget), THETA)
  end procedure cq_cphase

  module procedure cq_crotx !(qr, ctrl, qtarget, THETA) result(status) 
    status = crotx(get_qubit_at(qr, ctrl), get_qubit_at(qr, qtarget), THETA)
  end procedure cq_crotx

  module procedure cq_croty !(qr, ctrl, qtarget, THETA) result(status) 
    status = croty(get_qubit_at(qr, ctrl), get_qubit_at(qr, qtarget), THETA)
  end procedure cq_croty

  module procedure cq_crotz !(qr, ctrl, qtarget, THETA) result(status) 
    status = crotz(get_qubit_at(qr, ctrl), get_qubit_at(qr, qtarget), THETA)
  end procedure cq_crotz

  module procedure cq_chadamard !(qr, ctrl, qtarget) result(status) 
    status = chadamard(get_qubit_at(qr, ctrl), get_qubit_at(qr, qtarget))
  end procedure cq_chadamard

  module procedure cq_cunitary !(qr, ctrl, qtarget, THETA, PHI, LAMBDA) result(status) 
    status = cunitary(get_qubit_at(qr, ctrl), get_qubit_at(qr, qtarget), THETA, PHI, LAMBDA)
  end procedure cq_cunitary

  module procedure cq_swap !(qr, a, b) result(status) 
    status = swap(get_qubit_at(qr, a), get_qubit_at(qr, b))
  end procedure cq_swap

  module procedure cq_ccpaulix !(qr, ctrl_a, ctrl_b, qtarget) result(status) 
    status = ccpaulix(get_qubit_at(qr, ctrl_a), get_qubit_at(qr, ctrl_b), get_qubit_at(qr, qtarget))
  end procedure cq_ccpaulix

  module procedure cq_cswap !(qr, ctrl, a, b) result(status) 
    status = cswap(get_qubit_at(qr, ctrl), get_qubit_at(qr, a), get_qubit_at(qr, b))
  end procedure cq_cswap

end submodule qasm_gates
