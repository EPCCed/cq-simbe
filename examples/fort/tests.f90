program myfortran
use cq
use c_host_interface
implicit none

integer :: ireturn, freturn, alloc_status, free_status, reg_status, qrun_status
integer(kind=8) :: NQUBITS, NSHOTS, NMEASURE
type(qubit) :: qrc
integer, allocatable, target :: cr(:)

NQUBITS = 10
NSHOTS = 10
NMEASURE = NQUBITS

write(*,'(A)') 'before init'

ireturn = fcq_init(0)

write(*,'(A,I4)') 'cq_init returned: ',ireturn

alloc_status = cq_alloc_qureg(qrc, NQUBITS)

write(*,'(A,I4)') 'alloc_qureg returned: ', alloc_status

allocate(cr(NMEASURE * NSHOTS * 4))

CALL cq_init_creg(NMEASURE * NSHOTS, -1, cr)

write(*,'(A)') 'after init_creg'

reg_status = cq_register_qkern(c_funloc(foo))

write(*,'(A,I4)') 'after register_qkern: ', reg_status

qrun_status = cq_sm_qrun(c_funloc(foo), qrc, NQUBITS, cr, NMEASURE, NSHOTS)

write(*,'(A,I4)') 'after sm_qrun: ', qrun_status

CALL report_results(cr, NMEASURE, NSHOTS)

write(*,'(A)') 'after report results'


free_status = cq_free_qureg(qrc)

write(*,'(A,I4)') 'free_status returned: ', free_status

freturn = fcq_finalise(0)

write(*,'(A,I4)') 'cq_finalise returned: ',freturn

!write(*,'(A)') 'after finalise'

contains
  function foo(NQUBITS, qr, cr, reg) bind(C) result(status)
    use, intrinsic :: iso_c_binding, only: c_int, c_ptr, c_size_t
    implicit none
    integer(kind=8), value :: NQUBITS
    type(qubit) :: qr(NQUBITS)
    integer :: cr(NQUBITS)
    type(qkern_map), value :: reg
    integer :: i, status
    integer(kind=8) :: STATE_IDX

    !integer(c_size_t), value :: NQUBITS
    !type(c_ptr), value :: qr
    !type(c_ptr), value :: cr
    !type(c_ptr), value :: reg
    !integer(c_int) :: i, status
    !integer(c_size_t) :: STATE_IDX

    STATE_IDX = 0
    status = 1
    !CQ_REGISTER_FORT_KERNEL(reg)
    status = fcq_register_kernel("foo", reg)
    status = cq_set_qureg(qr, STATE_IDX, NQUBITS)
    do i = 1, NQUBITS
    !    status = hadamard(c_loc(qr(i)))
      status = i
    end do
    status = cq_measure_qureg(qr, NQUBITS, cr)
  end function foo

end program
