program zero_qft
use cq
use c_host_interface
implicit none

integer :: ireturn, freturn, alloc_status, free_status, reg_status, qrun_status
integer(kind=8) :: NQUBITS, NSHOTS, NMEASURE
type(qubit) :: qrc
integer, allocatable, target :: cr(:)
type(qkern) :: kernel

NQUBITS = 10
NSHOTS = 10
NMEASURE = NQUBITS

write(*,'(A)') 'before init'

ireturn = cq_init(0)

write(*,'(A,I4)') 'cq_init returned: ',ireturn

alloc_status = cq_alloc_qureg(qrc, NQUBITS)

write(*,'(A,I4)') 'alloc_qureg returned: ', alloc_status

allocate(cr(NMEASURE * NSHOTS * 4))

CALL cq_init_creg(NMEASURE * NSHOTS, -1, cr)

write(*,'(A)') 'after init_creg'

kernel%target = c_funloc(zero_state_qft)

reg_status = cq_register_qkern(kernel)

write(*,'(A,I4)') 'after register_qkern: ', reg_status

qrun_status = cq_sm_qrun(kernel, qrc, NQUBITS, cr, NMEASURE, NSHOTS)

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
    integer :: i, j
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

  function zero_state_qft(NQUBITS, qr, cr, reg) bind(C) result(status)
    implicit none
    integer(kind=8), value :: NQUBITS
    type(qubit), value :: qr
    integer :: cr(NQUBITS)
    type(qkern_map), value :: reg
    integer :: i, status
    integer(kind=8) :: STATE_IDX = 0
    status = cq_register_fort_kernel("plus_state_qft", reg)
    status = cq_set_qureg(qr, STATE_IDX, NQUBITS)

    call qft(NQUBITS, qr)

    status = cq_measure_qureg(qr, NQUBITS, cr)
  end function zero_state_qft

end program
