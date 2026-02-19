module c_host_interface
use iso_fortran_env, only: compiler_version

! ----- interface to the C function -----
use, intrinsic :: iso_c_binding
implicit none

interface
  function fortran_cq_init(VERBOSITY) bind(C, name="cq_init")
    use, intrinsic :: iso_c_binding, only: c_int
    implicit none
    integer(c_int), value :: VERBOSITY ! passes integer
    integer(c_int) :: fortran_cq_init
  end function fortran_cq_init

  function fortran_cq_finalise(VERBOSITY) bind(C, name="cq_finalise")
    use, intrinsic :: iso_c_binding, only: c_int
    implicit none
    integer(c_int), value :: VERBOSITY ! passes integer
    integer(c_int) :: fortran_cq_finalise
  end function fortran_cq_finalise

  !cq_status alloc_qubit(qubit ** qhp);
  function alloc_qubit(qhp) bind(C)
    use, intrinsic :: iso_c_binding, only: c_int, c_ptr
    implicit none
    type(c_ptr), intent(inout) :: qhp
    integer(c_int) :: alloc_qubit
  end function alloc_qubit

  !cq_status alloc_qubit(qubit ** qhp);
  function free_qubit(qhp) bind(C)
    use, intrinsic :: iso_c_binding, only: c_int, c_ptr
    implicit none
    type(c_ptr), intent(inout) :: qhp
    integer(c_int) :: free_qubit
  end function free_qubit

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
     use, intrinsic :: iso_c_binding, only: c_int, c_size_t, c_ptr, c_short
     implicit none
     integer(c_size_t), value :: LENGTH
     integer(c_int), value :: INIT_VAL
     integer(c_short) :: cr(0:LENGTH)
  end subroutine init_creg

  !cq_status register_qkern(qkern kernel)
  function register_qkern(kernel) bind(C)
     use, intrinsic :: iso_c_binding, only: c_int, c_ptr, c_funptr
     implicit none
     !type(c_ptr), value :: kernel
     type(c_funptr), intent(in), value :: kernel
     integer(c_int) :: register_qkern
  end function

  !cq_status s_qrun(qkern kernel, qubit * qrp, const size_t NQUBITS, cstate * const crp, const size_t NMEASURE);
  function s_qrun(kernel, qrp, NQUBITS, crp, NMEASURE) result(status) bind(C)
    use, intrinsic :: iso_c_binding, only: c_int, c_size_t, c_ptr, c_short
    implicit none
    type(c_ptr), value :: kernel
    type(c_ptr), value :: qrp
    integer(c_size_t), value :: NQUBITS
    integer(c_size_t), value :: NMEASURE
    integer(c_short) :: crp(0:NMEASURE)
    integer(c_int) :: status
  end function s_qrun

  !cq_status a_qrun(qkern kernel, qubit * qrp, const size_t NQUBITS, cstate * const crp, const size_t NMEASURE, cq_exec * const ehp);
  function a_qrun(kernel, qrp, NQUBITS, crp, NMEASURE, ehp) result(status) bind(C)
    use, intrinsic :: iso_c_binding, only: c_int, c_size_t, c_ptr, c_short
    implicit none
    type(c_ptr), value :: kernel
    type(c_ptr), value :: qrp
    integer(c_size_t), value :: NQUBITS
    integer(c_size_t), value :: NMEASURE
    integer(c_short) :: crp(0:NMEASURE)
    type(c_ptr), value :: ehp
    integer(c_int) :: status
  end function a_qrun

  !cq_status sm_qrun(qkern kernel, qubit * qrp, const size_t NQUBITS,
  ! cstate * const crp, const size_t NMEASURE, const size_t NSHOTS)
  function sm_qrun(kernel, qrp, NQUBITS, crp, NMEASURE, NSHOTS) bind(C)
     use, intrinsic :: iso_c_binding, only: c_int, c_size_t, c_ptr, c_short
     !import :: qubit
     implicit none
     type(c_ptr), value :: kernel
     type(c_ptr), value :: qrp
     integer(c_size_t), value :: NQUBITS
     integer(c_size_t), value :: NMEASURE
     integer(c_size_t), value :: NSHOTS
     integer(c_short) :: crp(0, NSHOTS*NMEASURE)
     integer(c_int) :: sm_qrun
   end function sm_qrun

  !cq_status am_qrun(qkern kernel, qubit * qrp, const size_t NQUBITS, 
  !  cstate * const crp, const size_t NMEASURE, const size_t NSHOTS,
  !  cq_exec * const ehp);
  function am_qrun(kernel, qrp, NQUBITS, crp, NMEASURE, NSHOTS, ehp) bind(C)
     use, intrinsic :: iso_c_binding, only: c_int, c_size_t, c_ptr, c_short
     implicit none
     type(c_ptr), value :: kernel
     type(c_ptr), value :: qrp
     integer(c_size_t), value :: NQUBITS
     integer(c_size_t), value :: NMEASURE
     integer(c_size_t), value :: NSHOTS
     integer(c_short) :: crp(0, NSHOTS*NMEASURE)
     type(c_ptr), value :: ehp
     integer(c_int) :: am_qrun
   end function am_qrun

!! Not implemented in CQ
!  !cq_status sb_qrun(qkern kernel, qubit * qrp, const size_t NQUBITS, 
!  !  cstate * const crp, const size_t NMEASURE, const backend_id BE);
!  function sb_qrun(kernel, qrp, NQUBITS, crp, NMEASURE, BE) result(status) bind(C)
!    use, intrinsic :: iso_c_binding, only: c_int, c_size_t, c_ptr, c_short
!    implicit none
!    type(c_ptr), value :: kernel
!    type(c_ptr), value :: qrp
!    integer(c_size_t), value :: NQUBITS
!    integer(c_size_t), value :: NMEASURE
!    integer(c_short) :: crp(0:NMEASURE)
!    integer(c_int) :: BE
!    integer(c_int) :: status
!  end function sb_qrun
!
!
  !cq_status ab_qrun(qkern kernel, qubit * qrp, const size_t NQUBITS, 
  !  cstate * const crp, const size_t NMEASURE, const backend_id BE,
  !  cq_exec * const ehp);
!  function ab_qrun(kernel, qrp, NQUBITS, crp, NMEASURE, BE, ehp) result(status) bind(C)
!    use, intrinsic :: iso_c_binding, only: c_int, c_size_t, c_ptr, c_short
!    implicit none
!    type(c_ptr), value :: kernel
!    type(c_ptr), value :: qrp
!    integer(c_size_t), value :: NQUBITS
!    integer(c_size_t), value :: NMEASURE
!    integer(c_short) :: crp(0:NMEASURE)
!    integer(c_int) :: BE
!    type(c_ptr), value :: ehp
!    integer(c_int) :: status
!  end function a_qrun
!
!
!cq_status smb_qrun(qkern kernel, qubit * qrp, const size_t NQUBITS, 
!  cstate * const crp, const size_t NMEASURE, const size_t NSHOTS, 
!  const backend_id BE);
!
!cq_status amb_qrun(qkern kernel, qubit * qrp, const size_t NQUBITS, 
!  cstate * const crp, const size_t NMEASURE, const size_t NSHOTS, 
!  const backend_id BE, cq_exec * const ehp);
!
!cq_status sp_qrun(pqkern kernel, void * kernpar, qubit * qrp, const size_t NQUBITS,
!  cstate * const crp, const size_t NMEASURE);
!
!cq_status ap_qrun(pqkern kernel, void * kernpar, qubit * qrp, const size_t NQUBITS,
!  cstate * const crp, const size_t NMEASURE, cq_exec * const ehp);
!
!cq_status smp_qrun(pqkern kernel, void * kernpar, qubit * qrp, const size_t NQUBITS,
!  cstate * const crp, const size_t NMEASURE, const size_t NSHOTS);
!
!cq_status amp_qrun(pqkern kernel, void * kernpar, qubit * qrp, const size_t NQUBITS,
!  cstate * const crp, const size_t NMEASURE, const size_t NSHOTS, 
!  cq_exec * const ehp);
!
!cq_status sbp_qrun(pqkern kernel, void * kernpar, qubit * qrp, const size_t NQUBITS, 
!  cstate * const crp, const size_t NMEASURE, const backend_id BE);
!
!cq_status abp_qrun(pqkern kernel, void * kernpar, qubit * qrp, const size_t NQUBITS, 
!  cstate * const crp, const size_t NMEASURE, 
!  const backend_id BE, cq_exec * const ehp);
!
!cq_status smbp_qrun(pqkern kernel, void * kernpar, qubit * qrp, const size_t NQUBITS, 
!  cstate * const crp, const size_t NMEASURE, const size_t NSHOTS, 
!  const backend_id BE);
!
!cq_status ambp_qrun(pqkern kernel, void * kernpar, qubit * qrp, const size_t NQUBITS, 
!  cstate * const crp, const size_t NMEASURE, const size_t NSHOTS, 
!  const backend_id BE, cq_exec * const ehp);
!
! Synchronisation

  !cq_status sync_qrun(cq_exec * const ehp);
  function sync_qrun(ehp) bind(C)
    use, intrinsic :: iso_c_binding, only: c_int, c_ptr
    implicit none
    type(c_ptr), value :: ehp
    integer(c_int) sync_qrun
  end function sync_qrun

  !cq_status wait_qrun(cq_exec * const ehp);
  function wait_qrun(ehp) bind(C)
    use, intrinsic :: iso_c_binding, only: c_int, c_ptr
    implicit none
    type(c_ptr), value :: ehp
    integer(c_int) wait_qrun
  end function wait_qrun

  !cq_status halt_qrun(cq_exec * const ehp);
  function halt_qrun(ehp) bind(C)
    use, intrinsic :: iso_c_binding, only: c_int, c_ptr
    implicit none
    type(c_ptr), value :: ehp
    integer(c_int) halt_qrun
  end function halt_qrun

end interface
! ----------
end  module c_host_interface
