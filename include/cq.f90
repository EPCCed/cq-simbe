module cq

use, intrinsic :: iso_c_binding
implicit none

! ------------------------------ DERIVED TYPES --------------------------------

type, bind(C) :: qubit
  type(c_ptr) :: this = c_null_ptr
end type qubit

type, bind(c) :: qubit_size
  integer(c_size_t) :: registry_index
  integer(c_size_t) :: offset
  integer(c_size_t) :: N
end type qubit_size

type :: qubit_ptr
  ! Wrapper type for implementing an array of pointers to qubit objects
  type(qubit), pointer :: target => null()
end type qubit_ptr

type, bind(C) :: qkern_map
  type(c_ptr) :: map
end type qkern_map

type :: qkern
  type(c_ptr) :: target = c_null_ptr
end type qkern

! ------------------------------ HOST OPERATIONS ------------------------------

interface
  module function cq_init(VERBOSITY) result(status)
    implicit none
    integer, value :: VERBOSITY
    integer :: status
  end function cq_init

  module function cq_finalise(VERBOSITY) result(status)
    implicit none
    integer, value :: VERBOSITY
    integer :: status
  end function cq_finalise

  module function cq_alloc_qureg(qrp, N) result(status)
    implicit none
    type(qubit), intent(inout) :: qrp
    integer(kind=8), value :: N
    integer :: status
  end function cq_alloc_qureg

  module function cq_free_qureg(qrp) result(status)
    implicit none
    type(qubit), intent(inout) :: qrp
    integer :: status
  end function cq_free_qureg

  module subroutine cq_init_creg(LENGTH, INIT_VAL, cr)
    implicit none
    integer(8) :: LENGTH
    integer :: INIT_VAL
    integer :: cr(0:LENGTH)
  end subroutine cq_init_creg

  module function cq_register_qkern(kernel) result(status)
     implicit none
     type(qkern), value :: kernel
     integer(c_int) :: status 
  end function cq_register_qkern

  module function cq_sm_qrun(kernel, qrp, NQUBITS, crp, NMEASURE, NSHOTS) result(status)
     implicit none
     type(qkern), value :: kernel
     integer(kind=8), value :: NQUBITS
     type(qubit), value :: qrp
     integer(kind=8), value :: NMEASURE
     integer(kind=8), value :: NSHOTS
     integer :: crp(0, NSHOTS*NMEASURE)
     integer :: status
   end function

end interface


! ----------------------------- DEVICE OPERATIONS -----------------------------

interface
  module function cq_register_fort_kernel(func_name, reg) result(status)
    implicit none
    character(*), intent(in) :: func_name(*)
    type(qkern_map), value :: reg
    integer :: status
  end function cq_register_fort_kernel

  module function cq_set_qureg(qr, STATE_IDX, NQUBITS) result(status)
    implicit none
    integer(kind=8), value :: NQUBITS
    type(qubit), value :: qr
    integer(kind=8), value :: STATE_IDX
    integer :: status
  end function cq_set_qureg

  module function cq_measure_qureg(qr, NQUBITS, cr) result(status)
      implicit none
      integer(kind=8) :: NQUBITS
      type(qubit), value :: qr
      integer :: cr(NQUBITS)
      integer :: status
  end function cq_measure_qureg
end interface

! --------------------------------- QASM GATES --------------------------------

interface

  module function cq_unitary(qh, qubit_idx, THETA, PHI, LAMBDA) result(status)
    implicit none
    type(qubit), value :: qh
    integer(kind=8) :: qubit_idx
    real(c_double) :: THETA
    real(c_double) :: PHI
    real(c_double) :: LAMBDA
    integer :: status
  end function cq_unitary

  module function cq_gphase(qh, qubit_idx, THETA) result(status)
    implicit none
    type(qubit), value :: qh
    integer(kind=8) :: qubit_idx
    real(c_double) :: THETA
    integer :: status
  end function cq_gphase

  module function cq_paulix(qh, qubit_idx) result(status) 
    implicit none
    type(qubit), value :: qh
    integer(kind=8) :: qubit_idx
    integer :: status
  end function cq_paulix

  module function cq_pauliy(qh, qubit_idx) result(status) 
    implicit none
    type(qubit), value :: qh
    integer(kind=8) :: qubit_idx
    integer :: status
  end function cq_pauliy

  module function cq_pauliz(qh, qubit_idx) result(status) 
    implicit none
    type(qubit), value :: qh
    integer(kind=8) :: qubit_idx
    integer :: status
  end function cq_pauliz

  module function cq_hadamard(qh, qubit_idx) result(status)
    implicit none
    type(qubit), value :: qh
    integer(kind=8) :: qubit_idx
    integer :: status
  end function cq_hadamard

  module function cq_sqrtz(qh, qubit_idx) result(status) 
    implicit none
    type(qubit), value :: qh
    integer(kind=8) :: qubit_idx
    integer :: status
  end function cq_sqrtz

  module function cq_sqrtzhc(qh, qubit_idx) result(status) 
    implicit none
    type(qubit), value :: qh
    integer(kind=8) :: qubit_idx
    integer :: status
  end function cq_sqrtzhc

  module function cq_sqrts(qh, qubit_idx) result(status) 
    implicit none
    type(qubit), value :: qh
    integer(kind=8) :: qubit_idx
    integer :: status
  end function cq_sqrts

  module function cq_sqrtshc(qh, qubit_idx) result(status) 
    implicit none
    type(qubit), value :: qh
    integer(kind=8) :: qubit_idx
    integer :: status
  end function cq_sqrtshc

  module function cq_sqrtx(qh, qubit_idx) result(status) 
    implicit none
    type(qubit), value :: qh
    integer(kind=8) :: qubit_idx
    integer :: status
  end function cq_sqrtx

  module function cq_rotx(qh, qubit_idx, THETA) result(status) 
    implicit none
    type(qubit), value :: qh
    integer(kind=8) :: qubit_idx
    real(c_double) :: THETA
    integer :: status
  end function cq_rotx

  module function cq_roty(qh, qubit_idx, THETA) result(status) 
    implicit none
    type(qubit), value :: qh
    integer(kind=8) :: qubit_idx
    real(c_double) :: THETA
    integer :: status
  end function cq_roty

  module function cq_rotz(qh, qubit_idx, THETA) result(status) 
    implicit none
    type(qubit), value :: qh
    integer(kind=8) :: qubit_idx
    real(c_double) :: THETA
    integer :: status
  end function cq_rotz
 
  module function cq_cpaulix(qr, ctrl, qtarget) result(status) 
    implicit none
    type(qubit), value :: qr
    integer(kind=8) :: ctrl
    integer(kind=8) :: qtarget
    integer :: status
  end function cq_cpaulix

  module function cq_cpauliy(qr, ctrl, qtarget) result(status) 
    implicit none
    type(qubit), value :: qr
    integer(kind=8) :: ctrl
    integer(kind=8) :: qtarget
    integer :: status
  end function cq_cpauliy

  module function cq_cpauliz(qr, ctrl, qtarget) result(status) 
    implicit none
    type(qubit), value :: qr
    integer(kind=8) :: ctrl
    integer(kind=8) :: qtarget
    integer :: status
  end function cq_cpauliz

  module function cq_cphase(qr, ctrl, qtarget, THETA) result(status) 
    implicit none
    type(qubit), value :: qr
    integer(kind=8) :: ctrl
    integer(kind=8) :: qtarget
    real(c_double) :: THETA
    integer :: status
  end function cq_cphase

  module function cq_crotx(qr, ctrl, qtarget, THETA) result(status) 
    implicit none
    type(qubit), value :: qr
    integer(kind=8) :: ctrl
    integer(kind=8) :: qtarget
    real(c_double) :: THETA
    integer :: status
  end function cq_crotx

  module function cq_croty(qr, ctrl, qtarget, THETA) result(status) 
    implicit none
    type(qubit), value :: qr
    integer(kind=8) :: ctrl
    integer(kind=8) :: qtarget
    real(c_double) :: THETA
    integer :: status

  end function cq_croty

  module function cq_crotz(qr, ctrl, qtarget, THETA) result(status) 
    implicit none
    type(qubit), value :: qr
    integer(kind=8) :: ctrl
    integer(kind=8) :: qtarget
    real(c_double) :: THETA
    integer :: status
  end function cq_crotz

  module function cq_chadamard(qr, ctrl, qtarget) result(status) 
    implicit none
    type(qubit), value :: qr
    integer(kind=8) :: ctrl
    integer(kind=8) :: qtarget
    integer :: status
  end function cq_chadamard

  module function cq_cunitary(qr, ctrl, qtarget, THETA, PHI, LAMBDA) result(status) 
    implicit none
    type(qubit), value :: qr
    integer(kind=8) :: ctrl
    integer(kind=8) :: qtarget
    real(c_double) :: THETA
    real(c_double) :: PHI
    real(c_double) :: LAMBDA
    integer :: status
  end function cq_cunitary

  module function cq_swap(qr, a, b) result(status) 
    implicit none
    type(qubit), value :: qr
    integer(kind=8) :: a
    integer(kind=8) :: b
    integer :: status
  end function cq_swap

  module function cq_ccpaulix(qr, ctrl_a, ctrl_b, qtarget) result(status) 
    implicit none
    type(qubit), value :: qr
    integer(kind=8) :: ctrl_a
    integer(kind=8) :: ctrl_b
    integer(kind=8) :: qtarget
    integer :: status
  end function cq_ccpaulix

  module function cq_cswap(qr, ctrl, a, b) result(status) 
    implicit none
    type(qubit), value :: qr
    integer(kind=8) :: ctrl
    integer(kind=8) :: a
    integer(kind=8) :: b
    integer :: status
  end function cq_cswap

end interface

end module
