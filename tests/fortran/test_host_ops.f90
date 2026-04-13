program test_host_ops
use cq
#include "cqf.h"

use test_utils
implicit none

call test_initialising_cq()
call test_resource_management()
call test_qkernel_reg()
call test_sync_qrun()
call test_executor_handle()
call test_async_qrun()
call test_finalising_cq()

contains
  function good_kernel(NQUBITS, qr, NMEASURE, cr, reg) result(status) bind(C)
    implicit none
    integer(kind=8), value :: NQUBITS
    type(qubit), value :: qr
    integer(kind=8), value :: NMEASURE
    integer(kind=2), intent(inout) :: cr(0:NQUBITS)
    !integer(kind=2), intent(inout) :: cr(:)
    type(qkern_map), value :: reg
    integer(kind=8) :: STATE_IDX = 0
    integer :: status
    CQ_REGISTER_KERNEL("good_kernel", reg)
    status = cq_set_qureg(qr, STATE_IDX, NQUBITS)
    status = cq_measure_qureg(qr, NQUBITS, cr)
  end function good_kernel

  subroutine test_initialising_cq()
    implicit none
    call test_header('Test initialising CQ')

    write(*, *) 'Test correct call to cq_init: '
    call assert(cq_init(0) == SUCCESS)
    write(*, *) 'Test repeated call to cq_init: '
    call assert(cq_init(4) == WARNING)
  end subroutine 


  subroutine test_resource_management()
    implicit none
    integer(kind=8) :: NQUBITS
    type(qubit) :: qhp
    type(qubit) :: qr
    NQUBITS = 10

    call test_header('Test resource management')

    write(*, *) 'Test freeing uninitialised qubit: '
    call assert(cq_free_qubit(qhp) == WARNING)

    write(*, *) 'Test allocating qubit: '
    call assert(cq_alloc_qubit(qhp) == SUCCESS)

    write(*, *) 'Test freeing qubit: '
    call assert(cq_free_qubit(qhp) == SUCCESS)
    
    write(*, *) 'Test freeing uninitialised qubit: '
    call assert(cq_free_qureg(qr) == WARNING)
    write(*, *) 'Test allocating qureg: '
    call assert(cq_alloc_qureg(qr, NQUBITS) == SUCCESS)
    write(*, *) 'Test freeing qureg: '
    call assert(cq_free_qureg(qr) == SUCCESS)
    
    NQUBITS = -1
    write(*, *) 'Test allocating qureg with negative number of qubits: '
    call assert(cq_alloc_qureg(qr, NQUBITS) == ERROR)

    !NQUBITS = 0
    !call assert(cq_alloc_qureg(qr, NQUBITS) == ERROR)
  end subroutine test_resource_management

  subroutine test_qkernel_reg()
    implicit none
    call test_header('Test quantum kernels')

    write(*, *) 'Test registering the quantum kernel: '
    call assert(cq_register_qkern(good_kernel) == SUCCESS)
  end subroutine test_qkernel_reg

  subroutine test_sync_qrun()
    implicit none
    integer(kind=8) :: NQUBITS
    integer(kind=8) :: NMEASURE
    integer(kind=8) :: NSHOTS
    
    type(qubit) :: qhp
    type(qubit) :: qr
    type(cq_exec) :: eh
    integer(kind=2), allocatable, target :: cr(:)
    integer(kind=2), allocatable, target :: cr_multi_shot(:)
    integer :: status
    NQUBITS = 10
    NMEASURE = 10
    NSHOTS = 10
    
    allocate(cr(NMEASURE))
    allocate(cr_multi_shot(NMEASURE * NSHOTS))

    call test_header('Test synchronous (blocking) executors')

    ! qr not init
    write(*, *) 'Test single shot offload on uninitialised qureg: '
    call assert(cq_s_qrun(good_kernel, qr, NQUBITS, cr, NMEASURE) == ERROR)

    write(*, *) 'Test single shot offload on initialised qureg: '
    NQUBITS = 10
    status = cq_alloc_qureg(qr, NQUBITS)
    call assert(cq_s_qrun(good_kernel, qr, NQUBITS, cr, NMEASURE) == SUCCESS)
    status = cq_free_qureg(qr)

    write(*, *) 'Test multi shot offload on initialised qureg: '
    status = cq_alloc_qureg(qr, NQUBITS)
    call assert(cq_sm_qrun(good_kernel, qr, NQUBITS, cr_multi_shot, NMEASURE, NSHOTS) == SUCCESS)
    status = cq_free_qureg(qr)
  end subroutine test_sync_qrun

  subroutine test_executor_handle()
    implicit none
    type(cq_exec) :: eh

    call test_header('Test executor handle')

    write(*, *) 'Test normal initialisation of executor handle: '
    call assert(cq_create_exec_handle(eh) == SUCCESS)
    write(*, *) 'Test repeated initialisation of executor handle: '
    call assert(cq_create_exec_handle(eh) == ERROR)
    write(*, *) 'Test freeing initialised executor handle: '
    call assert(cq_free_exec_handle(eh) == SUCCESS)

  end subroutine test_executor_handle

  subroutine test_async_qrun()
    implicit none
    integer(kind=8) :: NQUBITS
    integer(kind=8) :: NMEASURE
    integer(kind=8) :: NSHOTS
    
    type(qubit) :: qr
    type(cq_exec) :: eh
    integer(kind=2), allocatable, target :: cr(:)
    integer(kind=2), allocatable, target :: cr_multi_shot(:)
    integer :: status
    NQUBITS = 10
    NMEASURE = 10
    NSHOTS = 10
    
    allocate(cr(NMEASURE))
    allocate(cr_multi_shot(NMEASURE * NSHOTS))

    call test_header('Test asynchronous (non-blocking) executors')

    status = cq_alloc_qureg(qr, NQUBITS)
    ! eh is not allocated
    write(*, *) 'Test single shot offload on uninitialised executor handle: '
    call assert(cq_a_qrun(good_kernel, qr, NQUBITS, cr, NMEASURE, eh) == ERROR)

    ! freed eh
    write(*, *) 'Test single shot offload on freed executor handle: '
    status = cq_create_exec_handle(eh)
    status = cq_free_exec_handle(eh)
    call assert(cq_a_qrun(good_kernel, qr, NQUBITS, cr, NMEASURE, eh) == ERROR)

    write(*, *) 'Test single shot offload on initialised executor handle: '
    status = cq_create_exec_handle(eh)
    call assert(cq_a_qrun(good_kernel, qr, NQUBITS, cr, NMEASURE, eh) == SUCCESS)
    status = cq_free_qureg(qr)
    
    write(*, *) 'Test synchronisation of the executor: '
    call assert(cq_sync_qrun(eh) == SUCCESS)
    
    write(*, *) 'Test waiting on the executor: '
    status = cq_free_exec_handle(eh)
    status = cq_create_exec_handle(eh)
    status = cq_alloc_qureg(qr, NQUBITS)
    call assert(cq_a_qrun(good_kernel, qr, NQUBITS, cr, NMEASURE, eh) == SUCCESS)
    call assert(cq_wait_qrun(eh) == SUCCESS)
    status = cq_free_qureg(qr)

    write(*, *) 'Test halting the executor: '
    status = cq_free_exec_handle(eh)
    status = cq_create_exec_handle(eh)
    status = cq_alloc_qureg(qr, NQUBITS)
    call assert(cq_a_qrun(good_kernel, qr, NQUBITS, cr, NMEASURE, eh) == SUCCESS)
    call assert(cq_halt_qrun(eh) == SUCCESS)
    status = cq_free_qureg(qr)

    write(*, *) 'Test multi shot offload on initialised executor handle: '
    status = cq_free_exec_handle(eh)
    status = cq_create_exec_handle(eh)
    status = cq_alloc_qureg(qr, NQUBITS)
    call assert(cq_am_qrun(good_kernel, qr, NQUBITS, cr_multi_shot, NMEASURE, NSHOTS, eh) == SUCCESS)
    status = cq_free_qureg(qr)

  end subroutine test_async_qrun

  subroutine test_finalising_cq()
    implicit none
    call test_header('Test finalising CQ')

    write(*, *) 'Test correct call to cq_finalise: '
    call assert(cq_finalise(0) == SUCCESS)
    write(*, *) 'Test repeated call to cq_finalise: '
    call assert(cq_finalise(0) == WARNING)

  end subroutine 

 
end program
