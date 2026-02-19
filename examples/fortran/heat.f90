! based on https://quantum-journal.org/papers/q-2024-12-12-1563/pdf/
program heat_eq

use cq
#include "cqf.h"
use example_utils
implicit none

integer :: ireturn, freturn, alloc_status, free_status, reg_status, qrun_status
integer(kind=8) :: NQUBITS, NSHOTS, NMEASURE
type(qubit) :: qrc
integer(kind=2), allocatable, target :: cr(:)

NQUBITS = 10
NSHOTS = 10
NMEASURE = NQUBITS

write(*,'(A)') 'before init'

ireturn = cq_init(0)

write(*,'(A,I4)') 'cq_init returned: ',ireturn

alloc_status = cq_alloc_qureg(qrc, NQUBITS)

write(*,'(A,I4)') 'alloc_qureg returned: ', alloc_status

allocate(cr(NMEASURE * NSHOTS))

CALL cq_init_creg(NMEASURE * NSHOTS, -1, cr)

write(*,'(A)') 'after init_creg'

reg_status = cq_register_qkern(V_heat)

write(*,'(A,I4)') 'after register_qkern: ', reg_status

qrun_status = cq_sm_qrun(V_heat, qrc, NQUBITS, cr, NMEASURE, NSHOTS)

write(*,'(A,I4)') 'after sm_qrun: ', qrun_status

CALL report_results(cr, NMEASURE, NSHOTS)

write(*,'(A)') 'after report results'

free_status = cq_free_qureg(qrc)

write(*,'(A,I4)') 'free_status returned: ', free_status

freturn = cq_finalise(0)

write(*,'(A,I4)') 'cq_finalise returned: ',freturn

write(*,'(A)') 'after finalise'

contains

  subroutine qft(NQUBITS, qr)
    implicit none
    integer(kind=8), value :: NQUBITS
    type(qubit), value :: qr
    integer(kind=8) :: i, j
    integer :: status
    real(8), parameter :: PI = 3.1415926535897932384626433832795028841971694
    real(8) :: angle

    do i = 0, NQUBITS - 1
      status = cq_hadamard(qr, i)
      do j = i + 1, NQUBITS - 1 
        angle = PI / (2 ** j)
        status = cq_cphase(qr, j, i, angle)
      end do
    end do

    do i = 0, (NQUBITS / 2) - 1
      j = NQUBITS - (i + 1)
      status = cq_swap(qr, i, j)
    end do
  end subroutine qft 

  subroutine iqft(NQUBITS, qr)
    implicit none
    integer(kind=8), value :: NQUBITS
    type(qubit), value :: qr
    integer(kind=8) :: i, j

    integer :: status
    real(8), parameter :: PI = 3.1415926535897932384626433832795028841971694
    real(8) :: angle

    do i = (NQUBITS / 2) - 1, 0, -1
      j = NQUBITS - (i + 1)
      status = cq_swap(qr, i, j)
    end do

    do i = NQUBITS - 1, 0, -1
      do j = NQUBITS - 1, i + 1, -1
        angle = PI / (2 ** j)
        status = cq_cphase(qr, j, i, angle)
      end do
      status = cq_hadamard(qr, i)
    end do

  end subroutine iqft 


  function W(qr, NQUBITS, tau, lambda) result(status)
    implicit none
    type(qubit), value :: qr
    integer(kind=8), value :: NQUBITS
    real(8), value :: tau
    real(8), value :: lambda
    integer(kind=8) :: i
    integer :: status
 
    do i = 0, NQUBITS - 1
      status = cq_cpaulix(qr, NQUBITS - 1, i)
    end do

    status = cq_gphase(qr, NQUBITS - 1, lambda)
    status = cq_hadamard(qr, NQUBITS - 1)
    if (NQUBITS == 2) then
      status = cq_crotz(qr, NQUBITS - 2, NQUBITS - 1, -2.0 * tau)
    else 
      status = cq_rotz(qr, NQUBITS - 1, -2.0 * tau)
    end if 
    status = cq_hadamard(qr, NQUBITS - 1)
    status = cq_gphase(qr, NQUBITS - 1, -lambda)

    do i = 0, NQUBITS - 1
      status = cq_cpaulix(qr, NQUBITS - 1, i)
    end do

  end function W

  function V(qr, NQUBITS, tau) result(status)
    implicit none
    type(qubit), value :: qr
    integer(kind=8), value :: NQUBITS
    real(8), value :: tau
    integer(kind=8) :: i
    real(8) :: lambda = 0.2
    integer :: status

    do i = 1, NQUBITS
      status = W(qr, i, tau, lambda) 
      status = cq_gphase(qr, i - 1, -2.0 * tau)
    end do
    ! check it out
  end function V

  function V_til(qr, NSITES, tau) result(status)
    implicit none
    type(qubit), value :: qr
    integer, value :: NSITES
    real(8), value :: tau
    integer(kind=8) :: NQUBITS = 2
    integer(kind=8) :: i
    integer :: status
 
    do i = 0, NSITES - 1
      status = V(qr, NQUBITS, tau)
    end do

  end function V_til

  function V_heat(NQUBITS, qr, cr, reg) bind(C) result(status) 
    implicit none
    integer(kind=8), value :: NQUBITS
    type(qubit), value :: qr
    integer(kind=2) :: cr(0:NQUBITS)
    type(qkern_map), value :: reg
    integer(kind=8) :: i, j
    integer :: status
    integer(kind=8) :: STATE_IDX = 0
    integer(kind=8) :: NREPS = 2
    real(8) :: tau = 0.01
    integer :: NSITES = 2
    integer :: NQUBIT_PER_SITE = 2
    integer(kind=8) :: VPOWER

    CQ_REGISTER_KERNEL("V_heat", reg)
    status = cq_set_qureg(qr, STATE_IDX, NQUBITS)
    do i = 0, NREPS - 1
      call qft(NQUBITS - (NSITES * NQUBIT_PER_SITE), qr)
      status = cq_cpaulix(qr, i + (NSITES * NQUBIT_PER_SITE), i)
      VPOWER = 2 ** i
      do j = 0, VPOWER - 1
        status = V_til(qr, NSITES, tau)
      end do

      call iqft(NQUBITS - (NSITES * NQUBIT_PER_SITE), qr)
    end do
 
    status = cq_measure_qureg(qr, NQUBITS, cr)
  end function V_heat

end program
