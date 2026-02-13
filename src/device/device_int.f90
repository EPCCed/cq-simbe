module c_device_interface
use iso_fortran_env, only: compiler_version
use c_host_interface

! TODO: Is there a reason why we import qubit type? do we need it or is c_ptr enough

! ----- interface to the C function -----
use, intrinsic :: iso_c_binding
implicit none

interface
  function insert_to_qkern_map(func_name, reg) bind(C)
    use, intrinsic :: iso_c_binding, only: c_char, c_int, c_ptr
    implicit none
    character(kind=c_char), intent(in) :: func_name(*)
    type(c_ptr), value :: reg
    integer(c_int) :: insert_to_qkern_map
  end function

  function cq_register_kernel(func_name, reg) bind(C)
    use, intrinsic :: iso_c_binding, only: c_char, c_int, c_ptr
    implicit none
    character(kind=c_char), intent(in) :: func_name(*)
    type(c_ptr), value :: reg
    integer(c_int) :: cq_register_kernel
  end function

  function set_qureg(qr, STATE_IDX, NQUBITS) bind(C)
    use, intrinsic :: iso_c_binding, only: c_size_t, c_ptr, c_int
    !import :: qubit
    type(c_ptr), value :: qr
    integer(c_size_t), value :: NQUBITS
    !type(qubit) :: qr(NQUBITS)
    integer(c_size_t), value :: STATE_IDX
    integer(c_int) :: set_qureg
  end function

  function measure_qureg(qr, NQUBITS, cr) bind(C)
      use, intrinsic :: iso_c_binding, only: c_int, c_ptr, c_size_t
      !import :: qubit
      implicit none
      type(c_ptr), value :: qr
      integer(c_size_t), value :: NQUBITS
      !type(qubit) :: qr(NQUBITS)
      integer(c_int) :: cr(NQUBITS)
      integer(c_int) :: measure_qureg
  end function measure_qureg

! -------------------------------- QASM Gates --------------------------------
!cq_status unitary(qubit * qh, const double THETA, const double PHI, 
!  const double LAMBDA);

!cq_status gphase(qubit * qh, const double THETA);
!
!cq_status phase(qubit * qh, const double THETA);

  !cq_status paulix(qubit * qh);
  function paulix(qh) bind(C)
      use, intrinsic :: iso_c_binding, only: c_int, c_ptr
      !!import :: qubit
      implicit none
      type(c_ptr), value :: qh
      integer(c_int) :: paulix
  end function paulix

  !cq_status pauliy(qubit * qh);
  function pauliy(qh) bind(C)
      use, intrinsic :: iso_c_binding, only: c_int, c_ptr
      !!import :: qubit
      implicit none
      type(c_ptr), value :: qh
      integer(c_int) :: pauliy
  end function pauliy


  !cq_status pauliz(qubit * qh);
  function pauliz(qh) bind(C)
      use, intrinsic :: iso_c_binding, only: c_int, c_ptr
      !!import :: qubit
      implicit none
      type(c_ptr), value :: qh
      integer(c_int) :: pauliz
  end function pauliz

  !cq_status hadamard(qubit * qh);
  function hadamard(qh) bind(C)
      use, intrinsic :: iso_c_binding, only: c_int, c_ptr
      !import :: qubit
      implicit none
      !type(qubit) :: qh
      type(c_ptr), value :: qh
      integer(c_int) :: hadamard
  end function hadamard

  !cq_status sqrtz(qubit * qh);
  function sqrtz(qh) bind(C)
      use, intrinsic :: iso_c_binding, only: c_int, c_ptr
      !!import :: qubit
      implicit none
      type(c_ptr), value :: qh
      integer(c_int) :: sqrtz
  end function sqrtz

  !cq_status sqrtzhc(qubit * qh);
  function sqrtzhc(qh) bind(C)
      use, intrinsic :: iso_c_binding, only: c_int, c_ptr
      !!import :: qubit
      implicit none
      type(c_ptr), value :: qh
      integer(c_int) :: sqrtzhc
  end function sqrtzhc


  !cq_status sqrts(qubit * qh);
  function sqrts(qh) bind(C)
      use, intrinsic :: iso_c_binding, only: c_int, c_ptr
      !!import :: qubit
      implicit none
      type(c_ptr), value :: qh
      integer(c_int) :: sqrts
  end function sqrts


  !cq_status sqrtshc(qubit * qh);
  function sqrtshc(qh) bind(C)
      use, intrinsic :: iso_c_binding, only: c_int, c_ptr
      !!import :: qubit
      implicit none
      type(c_ptr), value :: qh
      integer(c_int) :: sqrtshc
  end function sqrtshc


  !cq_status sqrtx(qubit * qh);
  function sqrtx(qh) bind(C)
      use, intrinsic :: iso_c_binding, only: c_int, c_ptr
      !!import :: qubit
      implicit none
      type(c_ptr), value :: qh
      integer(c_int) :: sqrtx
  end function sqrtx

!cq_status rotx(qubit * qh, const double THETA);
!
!cq_status roty(qubit * qh, const double THETA);
!
!cq_status rotz(qubit * qh, const double THETA);
!
!cq_status cpaulix(qubit * ctrl, qubit * target);
!
!cq_status cpauliy(qubit * ctrl, qubit * target);
!
!cq_status cpauliz(qubit * ctrl, qubit * target);
!
!cq_status cphase(qubit * ctrl, qubit * target, const double THETA);
!
!cq_status crotx(qubit * ctrl, qubit *  target, const double THETA);
!
!cq_status croty(qubit * ctrl, qubit *  target, const double THETA);
!
!cq_status crotz(qubit * ctrl, qubit *  target, const double THETA);
!
!cq_status chadamard(qubit * ctrl, qubit * target);
!
!cq_status cunitary(qubit * ctrl, qubit * target, const double THETA, 
!  const double PHI, const double LAMBDA);
!
!cq_status swap(qubit * a, qubit * b);
!
!cq_status ccpaulix(qubit * ctrl_a, qubit * ctrl_b, qubit * target);
!
!cq_status cswap(qubit * ctrl, qubit * a, qubit * b);



end interface

end module c_device_interface
