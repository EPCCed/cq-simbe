module cq

use, intrinsic :: iso_c_binding
implicit none

! ------------------------------ DERIVED TYPES --------------------------------

type, bind(C) :: qubit
  type(c_ptr) :: this = c_null_ptr
end type qubit

type, bind(C) :: qubit_t
  integer(c_size_t) :: registry_index
  integer(c_size_t) :: offset
  integer(c_size_t) :: N
end type qubit_t

type :: qubit_ptr
  ! Wrapper type for implementing an array of pointers to qubit objects
  type(qubit), pointer :: target => null()
end type qubit_ptr

type, bind(C) :: qkern_map
  type(c_ptr) :: map
end type qkern_map

type, bind(C) :: cq_exec
  type(c_ptr) :: this = c_null_ptr
end type cq_exec

! ------------------------------ HOST OPERATIONS ------------------------------
abstract interface
  function qkern(NQUBITS, qr, NMEASURE, cr, reg) result(status) bind(C)
    use iso_c_binding, only: c_int, c_size_t, c_short
    import :: qubit, qkern_map
    implicit none
    integer(c_size_t), value :: NQUBITS
    type(qubit), value :: qr
    integer(c_size_t), value :: NMEASURE
    integer(c_short), intent(inout) :: cr(0:NMEASURE)
    type(qkern_map), value :: reg
    integer(c_int) :: status
  end function qkern
end interface

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

! Resource management

  module function cq_alloc_qubit(qhp) result(status)
    implicit none
    type(qubit), intent(inout) :: qhp
    integer :: status
  end function cq_alloc_qubit

  module function cq_free_qubit(qhp) result(status)
    implicit none
    type(qubit), intent(inout) :: qhp
    integer :: status
  end function cq_free_qubit

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
    integer(kind=8), value :: LENGTH
    integer, value :: INIT_VAL
    integer(c_short), intent(inout) :: cr(0:LENGTH)
  end subroutine cq_init_creg

! Synchronisation

  module function cq_sync_qrun(ehp) result(status)
    implicit none
    type(cq_exec), value :: ehp
    integer :: status
  end function

  module function cq_wait_qrun(ehp) result(status)
    implicit none
    type(cq_exec), value :: ehp
    integer :: status
  end function

  module function cq_halt_qrun(ehp) result(status)
    implicit none
    type(cq_exec), value :: ehp
    integer :: status
  end function

! Executor management
  module function cq_create_exec_handle(ehp) result(status)
    implicit none
    type(cq_exec), intent(inout) :: ehp
    integer :: status
  end function cq_create_exec_handle

  module function cq_free_exec_handle(ehp) result(status)
    implicit none
    type(cq_exec), intent(inout) :: ehp
    integer :: status
  end function cq_free_exec_handle

end interface


! ----------------------------- DEVICE OPERATIONS -----------------------------


interface
  module function cq_register_fort_kernel(func_name, reg) result(status)
    implicit none
    character(*), intent(in) :: func_name(*)
    type(qkern_map), value :: reg
    integer :: status
  end function cq_register_fort_kernel

! Resource management
  module function cq_set_qubit(qh, qidx, cs) result(status)
    implicit none
    type(qubit), value :: qh
    integer(kind=8), value :: qidx
    integer(kind=2), value :: cs
    integer :: status
  end function cq_set_qubit

  module function cq_set_qureg(qr, STATE_IDX, NQUBITS) result(status)
    implicit none
    integer(kind=8), value :: NQUBITS
    type(qubit), value :: qr
    integer(kind=8), value :: STATE_IDX
    integer :: status
  end function cq_set_qureg

  module function cq_set_qureg_cstate(qr, CR, N) result(status)
    implicit none
    integer(kind=8), value :: N
    type(qubit), value :: qr
    integer(kind=2), intent(inout) :: CR(0:N)
    integer :: status
  end function cq_set_qureg_cstate

! Control
  module function cq_qabort(STATUS) result(result)
    implicit none
    integer, value :: STATUS
    integer :: result
  end function cq_qabort

! Measurements
  module function cq_dmeasure_qubit(qbp, qidx, csp) result(status)
    implicit none
    type(qubit), value :: qbp
    integer(kind=8), value :: qidx
    integer(kind=2), target :: csp
    integer :: status
  end function cq_dmeasure_qubit

  module function cq_dmeasure_qureg(qr, NQUBITS, cr) result(status)
    implicit none
    integer(kind=8), value :: NQUBITS
    type(qubit), value :: qr
    ! this is slice to possibly larger cr so inout?
    integer(kind=2), intent(inout) :: cr(0:NQUBITS)
    integer :: status
  end function cq_dmeasure_qureg

  module function cq_dmeasure(qr, NQUBITS, TARGETS, NTARGETS, cr) result(status)
    implicit none
    integer(kind=8), value :: NQUBITS
    integer(kind=8), value :: NTARGETS
    integer(kind=2), intent(inout) :: cr(0:NTARGETS)
    integer(kind=8), intent(in) :: TARGETS(0:NTARGETS)
    type(qubit), value :: qr
    integer :: status
  end function cq_dmeasure

  module function cq_measure_qubit(qbp, qidx, csp) result(status)
    implicit none
    type(qubit), value :: qbp
    integer(kind=8), value :: qidx
    integer(kind=2), target :: csp
    integer :: status
  end function cq_measure_qubit

  module function cq_measure_qureg(qr, NQUBITS, cr) result(status)
    implicit none
    integer(kind=8), value :: NQUBITS
    type(qubit), value :: qr
    integer(kind=2), intent(inout) :: cr(0:NQUBITS)
    integer :: status
  end function cq_measure_qureg

  module function cq_measure(qr, NQUBITS, TARGETS, NTARGETS, cr) result(status)
    implicit none
    integer(kind=8), value :: NQUBITS
    integer(kind=8), value :: NTARGETS
    integer(kind=2), intent(inout) :: cr(0:NTARGETS)
    integer(kind=8), intent(in) :: TARGETS(0:NTARGETS)
    type(qubit), value :: qr
    integer :: status
  end function cq_measure

end interface

! --------------------------------- QASM GATES --------------------------------

interface

  module function cq_unitary(qh, qubit_idx, THETA, PHI, LAMBDA) result(status)
    implicit none
    type(qubit), value :: qh
    integer(kind=8), value :: qubit_idx
    real(c_double), value :: THETA
    real(c_double), value :: PHI
    real(c_double), value :: LAMBDA
    integer :: status
  end function cq_unitary

  module function cq_gphase(qh, qubit_idx, THETA) result(status)
    implicit none
    type(qubit), value :: qh
    integer(kind=8), value :: qubit_idx
    real(c_double), value :: THETA
    integer :: status
  end function cq_gphase

  module function cq_phase(qh, qubit_idx, THETA) result(status)
    implicit none
    type(qubit), value :: qh
    integer(kind=8), value :: qubit_idx
    real(c_double), value :: THETA
    integer :: status
  end function cq_phase

  module function cq_paulix(qh, qubit_idx) result(status) 
    implicit none
    type(qubit), value :: qh
    integer(c_size_t), value :: qubit_idx
    integer :: status
  end function cq_paulix

  module function cq_pauliy(qh, qubit_idx) result(status) 
    implicit none
    type(qubit), value :: qh
    integer(kind=8), value :: qubit_idx
    integer :: status
  end function cq_pauliy

  module function cq_pauliz(qh, qubit_idx) result(status) 
    implicit none
    type(qubit), value :: qh
    integer(kind=8), value :: qubit_idx
    integer :: status
  end function cq_pauliz

  module function cq_hadamard(qh, qubit_idx) result(status)
    implicit none
    type(qubit), value :: qh
    integer(kind=8), value :: qubit_idx
    integer :: status
  end function cq_hadamard

  module function cq_sqrtz(qh, qubit_idx) result(status) 
    implicit none
    type(qubit), value :: qh
    integer(kind=8), value :: qubit_idx
    integer :: status
  end function cq_sqrtz

  module function cq_sqrtzhc(qh, qubit_idx) result(status) 
    implicit none
    type(qubit), value :: qh
    integer(kind=8), value :: qubit_idx
    integer :: status
  end function cq_sqrtzhc

  module function cq_sqrts(qh, qubit_idx) result(status) 
    implicit none
    type(qubit), value :: qh
    integer(kind=8), value :: qubit_idx
    integer :: status
  end function cq_sqrts

  module function cq_sqrtshc(qh, qubit_idx) result(status) 
    implicit none
    type(qubit), value :: qh
    integer(kind=8), value :: qubit_idx
    integer :: status
  end function cq_sqrtshc

  module function cq_sqrtx(qh, qubit_idx) result(status) 
    implicit none
    type(qubit), value :: qh
    integer(kind=8), value :: qubit_idx
    integer :: status
  end function cq_sqrtx

  module function cq_rotx(qh, qubit_idx, THETA) result(status) 
    implicit none
    type(qubit), value :: qh
    integer(kind=8), value :: qubit_idx
    real(c_double), value :: THETA
    integer :: status
  end function cq_rotx

  module function cq_roty(qh, qubit_idx, THETA) result(status) 
    implicit none
    type(qubit), value :: qh
    integer(kind=8), value :: qubit_idx
    real(c_double), value :: THETA
    integer :: status
  end function cq_roty

  module function cq_rotz(qh, qubit_idx, THETA) result(status) 
    implicit none
    type(qubit), value :: qh
    integer(kind=8), value :: qubit_idx
    real(c_double), value :: THETA
    integer :: status
  end function cq_rotz
 
  module function cq_cpaulix(qr, ctrl, qtarget) result(status) 
    implicit none
    type(qubit), value :: qr
    integer(kind=8), value :: ctrl
    integer(kind=8), value :: qtarget
    integer :: status
  end function cq_cpaulix

  module function cq_cpauliy(qr, ctrl, qtarget) result(status) 
    implicit none
    type(qubit), value :: qr
    integer(kind=8), value :: ctrl
    integer(kind=8), value :: qtarget
    integer :: status
  end function cq_cpauliy

  module function cq_cpauliz(qr, ctrl, qtarget) result(status) 
    implicit none
    type(qubit), value :: qr
    integer(kind=8), value :: ctrl
    integer(kind=8), value :: qtarget
    integer :: status
  end function cq_cpauliz

  module function cq_cphase(qr, ctrl, qtarget, THETA) result(status) 
    implicit none
    type(qubit), value :: qr
    integer(kind=8), value :: ctrl
    integer(kind=8), value :: qtarget
    real(c_double), value :: THETA
    integer :: status
  end function cq_cphase

  module function cq_crotx(qr, ctrl, qtarget, THETA) result(status) 
    implicit none
    type(qubit), value :: qr
    integer(kind=8), value :: ctrl
    integer(kind=8), value :: qtarget
    real(c_double), value :: THETA
    integer :: status
  end function cq_crotx

  module function cq_croty(qr, ctrl, qtarget, THETA) result(status) 
    implicit none
    type(qubit), value :: qr
    integer(kind=8), value :: ctrl
    integer(kind=8), value :: qtarget
    real(c_double), value :: THETA
    integer :: status

  end function cq_croty

  module function cq_crotz(qr, ctrl, qtarget, THETA) result(status) 
    implicit none
    type(qubit), value :: qr
    integer(kind=8), value :: ctrl
    integer(kind=8), value :: qtarget
    real(c_double), value :: THETA
    integer :: status
  end function cq_crotz
  
  module function cq_chadamard(qr, ctrl, qtarget) result(status) 
    implicit none
    type(qubit), value :: qr
    integer(kind=8), value :: ctrl
    integer(kind=8), value :: qtarget
    integer :: status
  end function cq_chadamard

  module function cq_cunitary(qr, ctrl, qtarget, THETA, PHI, LAMBDA) result(status) 
    implicit none
    type(qubit), value :: qr
    integer(kind=8), value :: ctrl
    integer(kind=8), value :: qtarget
    real(c_double), value :: THETA
    real(c_double), value :: PHI
    real(c_double), value :: LAMBDA
    integer :: status
  end function cq_cunitary

  module function cq_swap(qr, a, b) result(status) 
    implicit none
    type(qubit), value :: qr
    integer(kind=8), value :: a
    integer(kind=8), value :: b
    integer :: status
  end function cq_swap

  module function cq_ccpaulix(qr, ctrl_a, ctrl_b, qtarget) result(status) 
    implicit none
    type(qubit), value :: qr
    integer(kind=8), value :: ctrl_a
    integer(kind=8), value :: ctrl_b
    integer(kind=8), value :: qtarget
    integer :: status
  end function cq_ccpaulix

  module function cq_cswap(qr, ctrl, a, b) result(status) 
    implicit none
    type(qubit), value :: qr
    integer(kind=8), value :: ctrl
    integer(kind=8), value :: a
    integer(kind=8), value :: b
    integer :: status
  end function cq_cswap

end interface

contains
  ! -------------------- Kernel-calling function definitions ------------------
  function cq_register_qkern(kernel) result(status)
    use, intrinsic :: iso_c_binding, only: c_int
    use c_host_interface
    implicit none
    procedure(qkern) :: kernel
    integer(c_int) :: status 
    status = register_qkern(c_funloc(kernel))
  end function cq_register_qkern

  function cq_s_qrun(kernel, qrp, NQUBITS, crp, NMEASURE) result(status)
    use c_host_interface
    implicit none
    procedure(qkern) :: kernel
    integer(kind=8), value :: NQUBITS
    type(qubit), value :: qrp
    integer(kind=8), value :: NMEASURE
    integer(c_short), intent(inout) :: crp(0:NMEASURE)
    integer :: status
    status = s_qrun(c_funloc(kernel), qrp%this, NQUBITS, crp, NMEASURE)
  end function

  function cq_a_qrun(kernel, qrp, NQUBITS, crp, NMEASURE, ehp) result(status)
    use c_host_interface
    implicit none
    procedure(qkern) :: kernel
    integer(kind=8), value :: NQUBITS
    type(qubit), value :: qrp
    integer(kind=8), value :: NMEASURE
    integer(c_short), intent(inout) :: crp(0:NMEASURE)
    type(cq_exec), value :: ehp
    integer :: status
    status = a_qrun(c_funloc(kernel), qrp%this, NQUBITS, crp, NMEASURE, ehp%this)
  end function

  function cq_sm_qrun(kernel, qrp, NQUBITS, crp, NMEASURE, NSHOTS) result(status)
    use c_host_interface
    implicit none
    procedure(qkern) :: kernel
    integer(kind=8), value :: NQUBITS
    type(qubit), value :: qrp
    integer(kind=8), value :: NMEASURE
    integer(kind=8), value :: NSHOTS
    integer(c_short), intent(inout) :: crp(0:NSHOTS*NMEASURE)
    integer :: status
    status = sm_qrun(c_funloc(kernel), qrp%this, NQUBITS, crp, NMEASURE, NSHOTS)
   end function

  function cq_am_qrun(kernel, qrp, NQUBITS, crp, NMEASURE, NSHOTS, ehp) result(status)
    use c_host_interface
    implicit none
    procedure(qkern) :: kernel
    integer(kind=8), value :: NQUBITS
    type(qubit), value :: qrp
    integer(kind=8), value :: NMEASURE
    integer(kind=8), value :: NSHOTS
    integer(c_short), intent(inout) :: crp(0:NSHOTS*NMEASURE)
    type(cq_exec), value :: ehp
    integer :: status
    status = am_qrun(c_funloc(kernel), qrp%this, NQUBITS, crp, NMEASURE, NSHOTS, ehp%this)
  end function

end module
