submodule (cq) host_ops

use c_host_interface
use c_device_interface
implicit none

contains

! ------------------------------ HOST OPERATIONS ------------------------------

  module procedure cq_init !(VERBOSITY) result(status)
    status = fortran_cq_init(VERBOSITY)
  end procedure cq_init

  module procedure cq_finalise !(VERBOSITY) result(status)
    status = fortran_cq_finalise(VERBOSITY)
  end procedure cq_finalise

  module procedure cq_alloc_qureg !(qrp, N) result(status)
    status = alloc_qureg(qrp%this, N)
  end procedure cq_alloc_qureg

  module procedure cq_free_qureg !(qrp) result(status)
    status = free_qureg(qrp%this)
  end procedure cq_free_qureg

  module procedure cq_init_creg !(LENGTH, INIT_VAL, cr) result(status)
    call init_creg(LENGTH, INIT_VAL, cr)
  end procedure cq_init_creg

  !module procedure cq_register_qkern !(kernel)
  !  !status = register_qkern(kernel%target)
  !  status = register_qkern(c_funloc(kernel))
  !end procedure cq_register_qkern


 ! function cq_register_qkern !(kernel)
 !   !status = register_qkern(kernel%target)
 !   status = register_qkern(c_funloc(kernel))
 ! end procedure cq_register_qkern


!  module procedure cq_sm_qrun !(kernel, qrp, NQUBITS, crp, NMEASURE, NSHOTS) bind(C) result(status)
!    status = sm_qrun(kernel%target, qrp%this, NQUBITS, crp, NMEASURE, NSHOTS)
!  end procedure

!  function bar(NQUBITS, qr, cr, reg) bind(C) result(status)
!    implicit none
!    integer(kind=8), value :: NQUBITS
!    type(qubit), value :: qr
!    integer :: cr(NQUBITS)
!    type(qkern_map), value :: reg
!    integer(kind=8) :: i
!    integer :: status
!    integer(kind=8) :: STATE_IDX = 0
!    status = cq_register_fort_kernel("bar", reg)
!    status = cq_set_qureg(qr, STATE_IDX, NQUBITS)
!
!    do i = 0, NQUBITS-1
!      status = cq_hadamard(qr, i)
!    end do
!
!    call qft(NQUBITS, qr)
!
!    status = cq_measure_qureg(qr, NQUBITS, cr)
!  end function bar


end submodule host_ops


