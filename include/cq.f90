module cq

use, intrinsic :: iso_c_binding
implicit none

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

! ------------------------------ HOST OPERATIONS ------------------------------

interface
  module function fcq_init(VERBOSITY) result(status)
    implicit none
    integer, value :: VERBOSITY
    integer :: status
  end function fcq_init

  module function fcq_finalise(VERBOSITY) result(status)
    implicit none
    integer, value :: VERBOSITY
    integer :: status
  end function fcq_finalise

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

  module function cq_register_qkern(kernel) result(status) bind(C)
     implicit none
     type(c_ptr), value :: kernel
     integer(c_int) :: status 
  end function cq_register_qkern

  module function cq_sm_qrun(kernel, qrp, NQUBITS, crp, NMEASURE, NSHOTS) bind(C) result(status)
     implicit none
     type(c_ptr), value :: kernel
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
  module function cq_hadamard(qh, qubit_idx) result(status)
    implicit none
    type(qubit), value :: qh
    integer :: qubit_idx
    integer :: status
  end function cq_hadamard
end interface

end module
