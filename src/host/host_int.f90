module c_host_interface
use iso_fortran_env, only: compiler_version

! ----- interface to the C function -----
use, intrinsic :: iso_c_binding
implicit none

type, bind(c) :: qubit_old
        integer(c_size_t) :: registry_index
        integer(c_size_t) :: offset
        integer(c_size_t) :: N
end type qubit_old
!
!type :: qubit_ptr
!  !! Wrapper type for implementing an array of pointers to qubit objects
!  type(qubit), pointer :: target
!end type qubit_ptr

interface
  function cq_init(VERBOSITY) bind(C)
    use, intrinsic :: iso_c_binding, only: c_int
    implicit none
    integer(c_int), value :: VERBOSITY ! passes integer
    integer(c_int) :: cq_init
  end function cq_init

  function cq_finalise(VERBOSITY) bind(C)
    use, intrinsic :: iso_c_binding, only: c_int
    implicit none
    integer(c_int), value :: VERBOSITY ! passes integer
    integer(c_int) :: cq_finalise
  end function cq_finalise

  !cq_status alloc_qureg(qubit ** qrp, size_t N);
  function alloc_qureg(qrp, N) bind(C)
    use, intrinsic :: iso_c_binding, only: c_int, c_size_t, c_ptr
    !import :: qubit
    implicit none
    type(c_ptr), intent(inout) :: qrp ! passes struct?
    integer(c_size_t), value :: N
    integer(c_int) :: alloc_qureg
  end function alloc_qureg

  !cq_status free_qureg(qubit ** qrp);
  function free_qureg(qrp) bind(C)
    use, intrinsic :: iso_c_binding, only: c_int, c_ptr
    !import ::qubit, c_ptr
    implicit none
    type(c_ptr), intent(inout) :: qrp ! passes struct?
    integer(c_int) :: free_qureg
  end function free_qureg

  !void init_creg(const size_t LENGTH, const cstate INIT_VAL, cstate * cr)
  subroutine init_creg(LENGTH, INIT_VAL, cr) bind(C)
     use, intrinsic :: iso_c_binding, only: c_int, c_size_t, c_ptr
     implicit none
     integer(c_size_t), value :: LENGTH
     integer(c_int), value :: INIT_VAL
     integer(c_int) :: cr(0:LENGTH)
  end subroutine init_creg

  !cq_status register_qkern(qkern kernel)
  function register_qkern(kernel) bind(C)
     use, intrinsic :: iso_c_binding, only: c_int, c_ptr
     implicit none
     type(c_ptr), value :: kernel
     integer(c_int) :: register_qkern
  end function

  !cq_status sm_qrun(qkern kernel, qubit * qrp, const size_t NQUBITS,
  ! cstate * const crp, const size_t NMEASURE, const size_t NSHOTS)
  function sm_qrun(kernel, qrp, NQUBITS, crp, NMEASURE, NSHOTS) bind(C)
     use, intrinsic :: iso_c_binding, only: c_int, c_size_t, c_ptr
     !import :: qubit
     implicit none
     type(c_ptr), value :: kernel
     type(c_ptr), value :: qrp
     integer(c_size_t), value :: NQUBITS
     integer(c_size_t), value :: NMEASURE
     integer(c_size_t), value :: NSHOTS
     integer(c_int) :: crp(0, NSHOTS*NMEASURE)
     integer(c_int) :: sm_qrun
   end function

  !Solution specific interfaces

  !report_results(cstate const * const CR, const size_t NMEASURE, const size_t NSHOTS)
  subroutine report_results(CR, NMEASURE, NSHOTS) bind(C)
     use, intrinsic :: iso_c_binding, only: c_int, c_size_t, c_ptr
     implicit none
     integer(c_size_t), value :: NMEASURE
     integer(c_size_t), value :: NSHOTS
     integer(c_int) :: CR(0, NSHOTS*NMEASURE)
  end subroutine


  !cq_status zero_init_full_qft(const size_t NQUBITS, qubit * qr, cstate * cr, qkern_map * reg)
  function zero_init_full_qft() bind(C)
      use, intrinsic :: iso_c_binding, only: c_int
      implicit none
      integer(c_int) :: zero_init_full_qft
  end function

  !cq_status plus_init_full_qft(const size_t NQUBITS, qubit * qr, cstate * cr, qkern_map * reg)
  function plus_init_full_qft() bind(C)
      use, intrinsic :: iso_c_binding, only: c_int
      implicit none
      integer(c_int) :: plus_init_full_qft
  end function


end interface
! ----------
end  module c_host_interface
