program plus_qft
use cq
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

reg_status = cq_register_qkern(plus_state_qft)

write(*,'(A,I4)') 'after register_qkern: ', reg_status

qrun_status = cq_sm_qrun(plus_state_qft, qrc, NQUBITS, cr, NMEASURE, NSHOTS)

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
        angle = PI / (2.0 ** j)
        status = cq_cphase(qr, j, i, angle)
      end do
    end do

    do i = 0, (NQUBITS / 2) - 1
      j = NQUBITS - (i + 1)
      status = cq_swap(qr, i, j)
    end do
  end subroutine qft 

  function plus_state_qft(NQUBITS, qr, cr, reg) bind(C) result(status)
    implicit none
    integer(kind=8), value :: NQUBITS
    type(qubit), value :: qr
    integer(kind=2) :: cr(0:NQUBITS)
    type(qkern_map), value :: reg
    integer(kind=8) :: i
    integer :: status
    integer(kind=8) :: STATE_IDX = 0
    status = cq_register_fort_kernel("plus_state_qft", reg)
    status = cq_set_qureg(qr, STATE_IDX, NQUBITS)

    do i = 0, NQUBITS-1
      status = cq_hadamard(qr, i)
    end do

    call qft(NQUBITS, qr)
    status = cq_measure_qureg(qr, NQUBITS, cr)
  end function plus_state_qft

  subroutine report_results(cr, NMEASURE, NSHOTS)
    implicit none
    integer(kind=8), value :: NMEASURE
    integer(kind=8), value :: NSHOTS
    integer(kind=2) :: cr(0:NMEASURE * NSHOTS)
    integer(kind=8) :: i, j

!    integer(kind=8) :: k
!    k = 0
!    do i = 0, NSHOTS-1, 1
!    do j = 0, NSHOTS-1, 1
!      cr(k) = k
!      k = k +1
!    end do
!    end do

    write(*,'(A)') 'Reporting measurement outcomes:'
    do i = 0, NSHOTS-1, 1
        write(*,'(A, I4, A)') 'Shot ', i, ': ' 
      do j = (i) * NMEASURE, (i+1) * NMEASURE - 2, 1
        write(*, '(I4, A)', advance='no') cr(j), ' '
      end do   
      write(*,'(I4, A)') cr((i+1) * NMEASURE - 1), ' '
    end do
  end subroutine report_results
end program
