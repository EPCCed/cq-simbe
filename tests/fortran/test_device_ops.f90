program test_device_ops
use cq
#include "cqf.h"

use test_utils
implicit none

integer :: status
status = cq_init(0)

call test_registering_kernel()
call test_setting_quantum_res()
call test_device_control()
call test_measurements()

status = cq_finalise(0)

contains

  subroutine test_registering_kernel()
  end subroutine

  function qureg_setter_kernel(NQUBITS, qr, NMEASURE, cr, reg) result(status) bind(C)
    implicit none
    integer(kind=8), value :: NQUBITS
    type(qubit), value :: qr
    integer(kind=8), value :: NMEASURE
    integer(kind=2), intent(inout) :: cr(0:NMEASURE)
    type(qkern_map), value :: reg
    integer(kind=8) :: STATE_IDX
    integer(kind=8) :: measured_state
    integer(kind=8) :: i, j
    integer :: status
    CQ_REGISTER_KERNEL("qureg_setter_kernel", reg)
    
    do i = 0, 32
      measured_state = 0
      STATE_IDX = i
      status = cq_set_qureg(qr, STATE_IDX, NQUBITS)
      status = cq_measure_qureg(qr, NQUBITS, cr)
      do j = 0, NQUBITS - 1
        measured_state = measured_state + (2 ** j) * cr(NQUBITS - 1 - j)
      end do
      call assert(STATE_IDX == measured_state)
    end do
  end function qureg_setter_kernel

  function set_qureg_with_creg_kernel(NQUBITS, qr, NMEASURE, cr_res, reg) result(status) bind(C)
    implicit none
    integer(kind=8), value :: NQUBITS
    type(qubit), value :: qr
    integer(kind=8), value :: NMEASURE
    integer(kind=2) :: cr_setter(0:NMEASURE)
    integer(kind=2), intent(inout) :: cr_res(0:NMEASURE)
    type(qkern_map), value :: reg
    integer(kind=8) :: i
    integer :: status
    CQ_REGISTER_KERNEL("set_qureg_with_creg_kernel", reg)

    cr_setter = (/0, 0, 1, 1, 0, 1/)
    status = cq_set_qureg_cstate(qr, cr_setter, NQUBITS)
    status = cq_measure_qureg(qr, NQUBITS, cr_res)
    do i = 0, NQUBITS - 1
      call assert(cr_res(i) == cr_setter(NQUBITS - 1 - i))
    end do

  end function

  function abort_kernel(NQUBITS, qr, NMEASURE, cr, reg) result(status) bind(C)
    implicit none
    integer(kind=8), value :: NQUBITS
    type(qubit), value :: qr
    integer(kind=8), value :: NMEASURE
    integer(kind=2), intent(inout) :: cr(0:NMEASURE)
    type(qkern_map), value :: reg
    integer(kind=8) :: STATE_IDX = 0
    integer(kind=8) :: qubit_idx = 0
    integer(kind=8) :: measured_state
    integer(kind=8) :: i
    integer :: status
    CQ_REGISTER_KERNEL("abort_kernel", reg)

    status = cq_set_qureg(qr, STATE_IDX, NQUBITS)
    status = cq_paulix(qr, qubit_idx)

    status = cq_measure_qureg(qr, NQUBITS, cr)
    if (cr(NQUBITS - 1) == 1) then
      status = cq_qabort(SUCCESS)
      call assert(status == EARLY_SUCCESS)
    end if

  end function

  function device_measurement_kernel(NQUBITS, qr, NMEASURE, cr, reg) result(status) bind(C)
    implicit none
    integer(kind=8), value :: NQUBITS
    type(qubit), value :: qr
    integer(kind=8), value :: NMEASURE
    integer(kind=2), intent(inout) :: cr(0:NMEASURE)
    integer(kind=8), parameter :: NTARGETS = 10
    integer(kind=8) :: TARGETS(0:NTARGETS)
    type(qkern_map), value :: reg
    integer(kind=8) :: STATE_IDX = 0
    integer(kind=8) :: qubit_idx = 0
    integer(kind=8) :: measured_state
    integer(kind=8) :: i
    integer :: status
    CQ_REGISTER_KERNEL("device_measurement_kernel", reg)
    status = cq_set_qureg(qr, STATE_IDX, NQUBITS)
    do i = 0, NQUBITS - 1, 2
      status = cq_paulix(qr, i)
    end do
    status = cq_dmeasure_qubit(qr, qubit_idx, cr(0))
    call assert(cr(0) == 1)
    status = cq_dmeasure_qureg(qr, NQUBITS, cr)
    do i = 0, NQUBITS - 1, 2
      call assert(cr(NQUBITS - 1 - i) == 1)
    end do

    ! reset creg and init targets
    do i = 0, NQUBITS - 1
      cr(i) = 0
      TARGETS(i) = i
    end do
    status = cq_dmeasure(qr, NQUBITS, TARGETS, NTARGETS, cr)
    do i = 0, NQUBITS - 1, 2
      call assert(cr(NQUBITS - 1 - i) == 1)
    end do

  end function

  function host_measurement_kernel(NQUBITS, qr, NMEASURE, cr, reg) result(status) bind(C)
    implicit none
    integer(kind=8), value :: NQUBITS
    type(qubit), value :: qr
    integer(kind=8), value :: NMEASURE
    integer(kind=2), intent(inout) :: cr(0:NMEASURE)
    integer(kind=8), parameter :: NTARGETS = 10
    integer(kind=8) :: TARGETS(0:NTARGETS)
    type(qkern_map), value :: reg
    integer(kind=8) :: STATE_IDX = 0
    integer(kind=8) :: qubit_idx = 0
    integer(kind=8) :: measured_state
    integer(kind=8) :: i
    integer :: status
    CQ_REGISTER_KERNEL("host_measurement_kernel", reg)
    status = cq_set_qureg(qr, STATE_IDX, NQUBITS)
    do i = 0, NQUBITS - 1, 2
      status = cq_paulix(qr, i)
    end do
    status = cq_measure_qubit(qr, qubit_idx, cr(0))
    call assert(cr(0) == 1)
    status = cq_measure_qureg(qr, NQUBITS, cr)
    do i = 0, NQUBITS - 1, 2
      call assert(cr(NQUBITS - 1 - i) == 1)
    end do

    ! reset creg and init targets
    do i = 0, NQUBITS - 1
      cr(i) = 0
      TARGETS(i) = i
    end do
    status = cq_measure(qr, NQUBITS, TARGETS, NTARGETS, cr)
    do i = 0, NQUBITS - 1, 2
      call assert(cr(NQUBITS - 1 - i) == 1)
    end do


  end function

  subroutine test_setting_quantum_res()
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

    call test_header('Test setting quantum resources')
    write(*, *) 'Test setting the qureg to different states: '
    status = cq_register_qkern(qureg_setter_kernel)
    status = cq_alloc_qureg(qr, NQUBITS)
    status = cq_s_qrun(qureg_setter_kernel, qr, NQUBITS, cr, NMEASURE)
    status = cq_free_qureg(qr)

    write(*, *) 'Test setting the qureg based on classical register: '
    call cq_init_creg(NMEASURE, 0, cr)
    status = cq_register_qkern(set_qureg_with_creg_kernel)
    status = cq_alloc_qureg(qr, NQUBITS)
    status = cq_s_qrun(set_qureg_with_creg_kernel, qr, NQUBITS, cr, NMEASURE)
    status = cq_free_qureg(qr)
  end subroutine

  subroutine test_device_control()
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
    call cq_init_creg(NMEASURE, 0, cr)
    call test_header('Test quantum kernel controls')
    status = cq_register_qkern(abort_kernel)
    status = cq_alloc_qureg(qr, NQUBITS)
    status = cq_s_qrun(abort_kernel, qr, NQUBITS, cr, NMEASURE)
    status = cq_free_qureg(qr)

  end subroutine

  subroutine test_measurements()
    implicit none
    integer(kind=8) :: NQUBITS
    integer(kind=8) :: NMEASURE
    integer(kind=8) :: NSHOTS

    type(qubit) :: qr
    integer(kind=2), allocatable, target :: cr(:)
    integer :: status
    NQUBITS = 10
    NMEASURE = 10
    NSHOTS = 10

    allocate(cr(NMEASURE))
    call cq_init_creg(NMEASURE, 0, cr)
    call test_header('Test quantum measurements')

    write(*, *) 'Test device (not-sync with host) measurements: '
    status = cq_register_qkern(device_measurement_kernel)
    status = cq_alloc_qureg(qr, NQUBITS)
    status = cq_s_qrun(device_measurement_kernel, qr, NQUBITS, cr, NMEASURE)
    status = cq_free_qureg(qr)

    write(*, *) 'Test host-synchronised measurements: '
    status = cq_register_qkern(host_measurement_kernel)
    status = cq_alloc_qureg(qr, NQUBITS)
    status = cq_s_qrun(host_measurement_kernel, qr, NQUBITS, cr, NMEASURE)
    status = cq_free_qureg(qr)

  end subroutine


end program
