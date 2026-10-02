module example_mpi_plus_qft_kernels
use cq
#include "cqf.h"
implicit none
contains

  subroutine qft(NQUBITS, qr, result_status)
    integer(kind=8), value :: NQUBITS
    type(qubit), value :: qr
    integer, optional, intent(out) :: result_status
    integer(kind=8) :: i, j
    integer :: status
    real(8), parameter :: PI = 3.1415926535897932384626433832795028841971694
    real(8) :: angle

    status = 0
    do i = 0, NQUBITS - 1
      status = cq_hadamard(qr, i)
      if (status /= 0) goto 900
      do j = i + 1, NQUBITS - 1
        angle = PI / (2.0 ** j)
        status = cq_cphase(qr, j, i, angle)
        if (status /= 0) goto 900
      end do
    end do
    do i = 0, (NQUBITS / 2) - 1
      j = NQUBITS - (i + 1)
      status = cq_swap(qr, i, j)
      if (status /= 0) goto 900
    end do
900 continue
    if (present(result_status)) result_status = status
  end subroutine qft

  function plus_state_qft(NQUBITS, qr, NMEASURE, cr, reg) bind(C) result(status)
    integer(kind=8), value :: NQUBITS
    type(qubit), value :: qr
    integer(kind=8), value :: NMEASURE
    integer(kind=2), intent(inout) :: cr(0:NMEASURE-1)
    type(qkern_map), value :: reg
    integer(kind=8) :: i
    integer :: status
    CQ_REGISTER_KERNEL("plus_state_qft", reg)
    if (NMEASURE < NQUBITS) then
      status = -1
      return
    end if
    status = cq_set_qureg(qr, 0_8, NQUBITS)
    if (status /= 0) return
    do i = 0, NQUBITS-1
      status = cq_hadamard(qr, i)
      if (status /= 0) return
    end do
    call qft(NQUBITS, qr, status)
    if (status /= 0) return
    status = cq_measure_qureg(qr, NQUBITS, cr)
  end function plus_state_qft

end module example_mpi_plus_qft_kernels

program mpi_plus_qft
use mpi
use cq
use example_mpi_plus_qft_kernels
#include "cqf.h"
use example_utils
implicit none
integer :: ireturn, freturn, reg_status, alloc_status, qrun_status, free_status, mem_status
integer :: failed = 0
integer(kind=8), parameter :: NQUBITS = 10, NSHOTS = 10, NMEASURE = NQUBITS
type(qubit) :: qrc
integer(kind=2), allocatable, target :: cr(:)
integer :: ierr, id, nproc, cq_comm, provided, any_failed
call MPI_Init_thread(MPI_THREAD_MULTIPLE, provided, ierr)
if (ierr /= MPI_SUCCESS) error stop 1
call MPI_Comm_size(MPI_COMM_WORLD, nproc, ierr)
if (ierr /= MPI_SUCCESS) call MPI_Abort(MPI_COMM_WORLD, 1, ierr)
call MPI_Comm_rank(MPI_COMM_WORLD, id, ierr)
if (ierr /= MPI_SUCCESS) call MPI_Abort(MPI_COMM_WORLD, 1, ierr)
if (provided < MPI_THREAD_MULTIPLE .or. nproc < 4) then
  if (id == 0) write(*,*) 'mpi_plus_qft requires MPI_THREAD_MULTIPLE and at least four ranks'
  call MPI_Finalize(ierr)
  error stop 1
end if
if (id >= 3) then
  call MPI_Comm_split(MPI_COMM_WORLD, 1, id, cq_comm, ierr)
else
  call MPI_Comm_split(MPI_COMM_WORLD, MPI_UNDEFINED, id, cq_comm, ierr)
end if
if (ierr /= MPI_SUCCESS) call MPI_Abort(MPI_COMM_WORLD, 1, ierr)
if (cq_comm /= MPI_COMM_NULL) then
  ireturn = cq_init_custom_mpi_comm(cq_comm, 1)
if (ireturn /= 0) then
  write(*,*) 'CQ initialization failed: ', ireturn
  failed = 1
else
  reg_status = cq_register_qkern(plus_state_qft)
  if (reg_status /= 0) then
    write(*,*) 'Kernel registration failed: ', reg_status
    failed = 1
  end if
  CQ_PROG_BEGIN()
  if (failed == 0) then
    alloc_status = cq_alloc_qureg(qrc, NQUBITS)
    if (alloc_status /= 0) then
      write(*,*) 'Qubit allocation failed: ', alloc_status
      failed = 1
    else
      allocate(cr(NMEASURE * NSHOTS), stat=mem_status)
      if (mem_status /= 0) then
        write(*,*) 'Classical result allocation failed: ', mem_status
        failed = 1
      else
        call cq_init_creg(NMEASURE * NSHOTS, -1, cr)
        qrun_status = cq_sm_qrun(plus_state_qft, qrc, NQUBITS, cr, NMEASURE, NSHOTS)
        if (qrun_status /= 0) then
          write(*,*) 'QFT execution failed: ', qrun_status
          failed = 1
        else
          call report_results(cr, NMEASURE, NSHOTS)
        end if
        deallocate(cr)
      end if
      free_status = cq_free_qureg(qrc)
      if (free_status /= 0) failed = 1
    end if
  end if
  CQ_PROG_END()
  freturn = cq_finalise(0)
  if (freturn /= 0) then
    write(*,*) 'CQ finalization failed: ', freturn
    failed = 1
  end if
end if
  call MPI_Comm_free(cq_comm, ierr)
  if (ierr /= MPI_SUCCESS) failed = 1
end if
call MPI_Allreduce(failed, any_failed, 1, MPI_INTEGER, MPI_MAX, MPI_COMM_WORLD, ierr)
if (ierr /= MPI_SUCCESS) call MPI_Abort(MPI_COMM_WORLD, 1, ierr)
call MPI_Finalize(ierr)
if (ierr /= MPI_SUCCESS .or. any_failed /= 0) error stop 1
end program mpi_plus_qft
