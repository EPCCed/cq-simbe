program test_qasm_gates
use cq
#include "cqf.h"

use test_utils
implicit none

integer :: status
status = cq_init(0)

call test_simple_gates()
call test_rotation_gates()
call test_control_gates()
call test_control_rotation_gates()
call test_multi_control_gates()

status = cq_finalise(0)

contains

  function simple_gates_kernel(NQUBITS, qr, cr, reg) result(status) bind(C)
    implicit none
    integer(kind=8), value :: NQUBITS
    type(qubit), value :: qr
    integer(kind=2) :: cr(0:NQUBITS)
    type(qkern_map), value :: reg
    integer(kind=8) :: STATE_IDX = 0
    integer(kind=8) :: measured_state
    integer(kind=8) :: i, j
    integer(kind=8) :: q = 1 
    integer :: status
    CQ_REGISTER_KERNEL("simple_gates_kernel", reg)
    status = cq_set_qureg(qr, STATE_IDX, NQUBITS)

    status = cq_paulix(qr, q)
    call assert(status == SUCCESS)

    status = cq_pauliy(qr, q)
    call assert(status == SUCCESS)

    status = cq_pauliz(qr, q)
    call assert(status == SUCCESS)

    status = cq_hadamard(qr, q)
    call assert(status == SUCCESS)

    status = cq_sqrtz(qr, q)
    call assert(status == SUCCESS)

    status = cq_sqrtzhc(qr, q)
    call assert(status == SUCCESS)
        
    status = cq_sqrts(qr, q)
    call assert(status == SUCCESS)

    status = cq_sqrtshc(qr, q)
    call assert(status == SUCCESS)

    status = cq_sqrtx(qr, q)
    call assert(status == SUCCESS)
 
  end function

  function rotation_gates_kernel(NQUBITS, qr, cr, reg) result(status) bind(C)
    implicit none
    integer(kind=8), value :: NQUBITS
    type(qubit), value :: qr
    integer(kind=2) :: cr(0:NQUBITS)
    type(qkern_map), value :: reg
    integer(kind=8) :: STATE_IDX = 0
    integer(kind=8) :: measured_state
    integer(kind=8) :: i, j
    integer(kind=8) :: q = 1 
    real(kind=8) :: THETA = 3.14
    integer :: status
    CQ_REGISTER_KERNEL("rotation_gates_kernel", reg)
    status = cq_set_qureg(qr, STATE_IDX, NQUBITS)

    status = cq_rotx(qr, q, THETA)
    call assert(status == SUCCESS)

    status = cq_roty(qr, q, THETA)
    call assert(status == SUCCESS)

    status = cq_rotz(qr, q, THETA)
    call assert(status == SUCCESS)

    status = cq_gphase(qr, q, THETA)
    call assert(status == SUCCESS)

    status = cq_phase(qr, q, THETA)
    call assert(status == SUCCESS)
 
  end function

  function control_gates_kernel(NQUBITS, qr, cr, reg) result(status) bind(C)
    implicit none
    integer(kind=8), value :: NQUBITS
    type(qubit), value :: qr
    integer(kind=2) :: cr(0:NQUBITS)
    type(qkern_map), value :: reg
    integer(kind=8) :: STATE_IDX = 0
    integer(kind=8) :: measured_state
    integer(kind=8) :: i, j
    integer(kind=8) :: c = 1 
    integer(kind=8) :: t = 2 
    integer :: status
    CQ_REGISTER_KERNEL("control_gates_kernel", reg)
    status = cq_set_qureg(qr, STATE_IDX, NQUBITS)

    status = cq_cpaulix(qr, c, t)
    call assert(status == SUCCESS)

    status = cq_cpauliy(qr, c, t)
    call assert(status == SUCCESS)

    status = cq_cpauliz(qr, c, t)
    call assert(status == SUCCESS)

    status = cq_chadamard(qr, c, t)
    call assert(status == SUCCESS)

    status = cq_swap(qr, c, t)
    call assert(status == SUCCESS)
 
  end function

  function control_rotation_gates(NQUBITS, qr, cr, reg) result(status) bind(C)
    implicit none
    integer(kind=8), value :: NQUBITS
    type(qubit), value :: qr
    integer(kind=2) :: cr(0:NQUBITS)
    type(qkern_map), value :: reg
    integer(kind=8) :: STATE_IDX = 0
    integer(kind=8) :: measured_state
    integer(kind=8) :: i, j
    integer(kind=8) :: c = 1 
    integer(kind=8) :: t = 2 
    real(kind=8) :: THETA = 3.14
    integer :: status

    CQ_REGISTER_KERNEL("control_rotation_gates", reg)
    status = cq_set_qureg(qr, STATE_IDX, NQUBITS)

    status = cq_crotx(qr, c, t, THETA)
    call assert(status == SUCCESS)

    status = cq_croty(qr, c, t, THETA)
    call assert(status == SUCCESS)

    status = cq_crotz(qr, c, t, THETA)
    call assert(status == SUCCESS)

    status = cq_cphase(qr, c, t, THETA)
    call assert(status == SUCCESS)
 
  end function

  function multi_control_gates_kernel(NQUBITS, qr, cr, reg) result(status) bind(C)
    implicit none
    integer(kind=8), value :: NQUBITS
    type(qubit), value :: qr
    integer(kind=2) :: cr(0:NQUBITS)
    type(qkern_map), value :: reg
    integer(kind=8) :: STATE_IDX = 0
    integer(kind=8) :: measured_state
    integer(kind=8) :: i, j
    integer(kind=8) :: c1 = 1 
    integer(kind=8) :: c2 = 2 
    integer(kind=8) :: t = 4
    integer(kind=8) :: a = 2
    integer(kind=8) :: b = 4
    integer :: status
    CQ_REGISTER_KERNEL("multi_control_gates_kernel", reg)
    status = cq_set_qureg(qr, STATE_IDX, NQUBITS)

    status = cq_ccpaulix(qr, c1, c2, t)
    call assert(status == SUCCESS)

    status = cq_cswap(qr, c1, a, b)
    call assert(status == SUCCESS)
 
  end function

  subroutine test_simple_gates()
    implicit none
    integer(kind=8) :: NQUBITS
    integer(kind=8) :: NMEASURE
    integer(kind=8) :: NSHOTS
    
    type(qubit) :: qhp
    type(qubit) :: qr
    integer(kind=2), allocatable, target :: cr(:)
    integer :: status
    NQUBITS = 10
    NMEASURE = 10
    NSHOTS = 10
    
    allocate(cr(NMEASURE))

    call test_header('Test simple (1 qubit) quantum gates')
    status = cq_register_qkern(simple_gates_kernel)
    status = cq_alloc_qureg(qr, NQUBITS)
    status = cq_s_qrun(simple_gates_kernel, qr, NQUBITS, cr, NMEASURE)
    status = cq_free_qureg(qr)

  end subroutine

  subroutine test_rotation_gates()
    implicit none
    integer(kind=8) :: NQUBITS
    integer(kind=8) :: NMEASURE
    integer(kind=8) :: NSHOTS
    
    type(qubit) :: qhp
    type(qubit) :: qr
    integer(kind=2), allocatable, target :: cr(:)
    integer :: status
    NQUBITS = 10
    NMEASURE = 10
    NSHOTS = 10
    
    allocate(cr(NMEASURE))

    call test_header('Test rotation quantum gates')
    status = cq_register_qkern(rotation_gates_kernel)
    status = cq_alloc_qureg(qr, NQUBITS)
    status = cq_s_qrun(rotation_gates_kernel, qr, NQUBITS, cr, NMEASURE)
    status = cq_free_qureg(qr)

  end subroutine

  subroutine test_control_gates()
    implicit none
    integer(kind=8) :: NQUBITS
    integer(kind=8) :: NMEASURE
    integer(kind=8) :: NSHOTS
    
    type(qubit) :: qhp
    type(qubit) :: qr
    integer(kind=2), allocatable, target :: cr(:)
    integer :: status
    NQUBITS = 10
    NMEASURE = 10
    NSHOTS = 10
    
    allocate(cr(NMEASURE))

    call test_header('Test controlled quantum gates')
    status = cq_register_qkern(control_gates_kernel)
    status = cq_alloc_qureg(qr, NQUBITS)
    status = cq_s_qrun(control_gates_kernel, qr, NQUBITS, cr, NMEASURE)
    status = cq_free_qureg(qr)

  end subroutine

  subroutine test_control_rotation_gates()
    implicit none
    integer(kind=8) :: NQUBITS
    integer(kind=8) :: NMEASURE
    integer(kind=8) :: NSHOTS
    
    type(qubit) :: qhp
    type(qubit) :: qr
    integer(kind=2), allocatable, target :: cr(:)
    integer :: status
    NQUBITS = 10
    NMEASURE = 10
    NSHOTS = 10
    
    allocate(cr(NMEASURE))

    call test_header('Test controlled rotation quantum gates')
    status = cq_register_qkern(control_rotation_gates)
    status = cq_alloc_qureg(qr, NQUBITS)
    status = cq_s_qrun(control_rotation_gates, qr, NQUBITS, cr, NMEASURE)
    status = cq_free_qureg(qr)


  end subroutine

  subroutine test_multi_control_gates()
    implicit none
    integer(kind=8) :: NQUBITS
    integer(kind=8) :: NMEASURE
    integer(kind=8) :: NSHOTS
    
    type(qubit) :: qhp
    type(qubit) :: qr
    integer(kind=2), allocatable, target :: cr(:)
    integer :: status
    NQUBITS = 10
    NMEASURE = 10
    NSHOTS = 10
    
    allocate(cr(NMEASURE))

    call test_header('Test simple (1 qubit) quantum gates')
    status = cq_register_qkern(multi_control_gates_kernel)
    status = cq_alloc_qureg(qr, NQUBITS)
    status = cq_s_qrun(multi_control_gates_kernel, qr, NQUBITS, cr, NMEASURE)
    status = cq_free_qureg(qr)

  end subroutine

end
