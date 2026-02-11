program myfortran
use  c_host_interface
use iso_c_binding, only : c_int, c_ptr, c_loc, c_funloc

implicit none

integer(c_int) :: ireturn, freturn, alloc_status, free_status, reg_status, qrun_status
integer(c_size_t) :: NQUBITS, NSHOTS, NMEASURE
type(c_ptr) :: qrc
integer, allocatable, target :: cr(:)
type(c_ptr) :: cr_ptr

!!cq_status plus_init_full_qft(const size_t NQUBITS, qubit * qr, cstate * cr, qkern_map * reg)


NQUBITS = 10
NSHOTS = 10
NMEASURE = NQUBITS

write(*,'(A)') 'before init'

ireturn = cq_init(0)

write(*,'(A,I4)') 'cq_init returned: ',ireturn

alloc_status = alloc_qureg(qrc, NQUBITS)

write(*,'(A,I4)') 'alloc_qureg returned: ', alloc_status

allocate(cr(NMEASURE * NSHOTS * 4))

write(*,'(A)') 'after allocate'

!cr_ptr = c_loc(cr)

write(*,'(A)') 'after c_loc'

CALL init_creg(NMEASURE * NSHOTS, -1, cr)

write(*,'(A)') 'after int_creg'

!reg_status = register_qkern(c_funloc(zero_init_full_qft))
!reg_status = register_qkern(c_funloc(plus_init_full_qft))
reg_status = register_qkern(c_funloc(foo))

write(*,'(A,I4)') 'after register_qkern: ', reg_status

!qrun_status = sm_qrun(c_funloc(zero_init_full_qft), qrc, NQUBITS, cr, NMEASURE, NSHOTS)
!qrun_status = sm_qrun(c_funloc(plus_init_full_qft), qrc, NQUBITS, cr, NMEASURE, NSHOTS)
qrun_status = sm_qrun(c_funloc(foo), qrc, NQUBITS, cr, NMEASURE, NSHOTS)

write(*,'(A,I4)') 'after sm_qrun: ', qrun_status

CALL report_results(cr, NMEASURE, NSHOTS)

write(*,'(A)') 'after report results'

free_status = free_qureg(qrc)

write(*,'(A,I4)') 'free_status returned: ', free_status

freturn = cq_finalise(0)

write(*,'(A,I4)') 'cq_finalise returned: ',freturn

write(*,'(A)') 'after finalise'

CONTAINS
  integer function foo(NQUBITS, qr, cr, reg) bind(C)
    use, intrinsic :: iso_c_binding, only: c_int, c_ptr, c_size_t
    implicit none
    integer(c_size_t), value :: NQUBITS
    type(qubit), target :: qr(NQUBITS)
    !type(c_ptr), value :: qr
    !type(c_ptr), value :: cr
    integer(c_int) :: cr(NQUBITS)
    type(c_ptr), value :: reg
    integer :: i, status
    integer(c_size_t) :: STATE_IDX
  
    STATE_IDX = 0
    status = 1
    !CQ_REGISTER_FORT_KERNEL(reg)
    status = insert_to_qkern_map("foo", reg)
    status = set_qureg(c_loc(qr), STATE_IDX, NQUBITS)
    do i = 1, NQUBITS
        status = hadamard(c_loc(qr(i)))
    end do
    status = measure_qureg(c_loc(qr), NQUBITS, cr)
    foo=status
  end function

end program
