module c_device_interface
use iso_fortran_env, only: compiler_version
use c_host_interface

! ----- interface to the C function -----
use, intrinsic :: iso_c_binding
implicit none

interface
  function insert_to_qkern_map(func_name, reg) bind(C, name="fort_insert_to_qkern_map")
    use, intrinsic :: iso_c_binding, only: c_char, c_int, c_ptr
    implicit none
    character(kind=c_char), intent(in) :: func_name(*)
    type(c_ptr), value :: reg
    integer(c_int) :: insert_to_qkern_map
  end function

  !cq_status set_qubit(qubit qh, cstate cs);
  function set_qubit(qh, cs) bind(C)
    use, intrinsic :: iso_c_binding, only: c_short, c_ptr, c_int
    implicit none
    type(c_ptr), value :: qh
    integer(c_short), value :: cs
    integer(c_int) :: set_qubit
  end function

  !cq_status set_qureg(qubit * qrp, const unsigned long long STATE_IDX, const size_t N);
  function set_qureg(qr, STATE_IDX, NQUBITS) bind(C)
    use, intrinsic :: iso_c_binding, only: c_size_t, c_ptr, c_int
    implicit none
    type(c_ptr), value :: qr
    integer(c_size_t), value :: NQUBITS
    integer(c_size_t), value :: STATE_IDX
    integer(c_int) :: set_qureg
  end function

  !cq_status set_qureg_cstate(qubit * qrp, cstate const * const CRP, const size_t N);
  function set_qureg_cstate(qr, CR, N) bind(C)
    use, intrinsic :: iso_c_binding, only: c_size_t, c_ptr, c_int, c_short
    implicit none
    type(c_ptr), value :: qr
    integer(c_size_t), value :: N
    integer(c_short) :: CR(0:N)
    integer(c_int) :: set_qureg_cstate
  end function

  !cq_status qabort(const int STATUS);
  function qabort(STATUS) bind(C)
    use, intrinsic :: iso_c_binding, only: c_int
    implicit none
    integer(c_int), value :: STATUS
    integer(c_int) :: qabort
  end function qabort

  !cq_status dmeasure_qubit(qubit * qbp, cstate * csp);
  function dmeasure_qubit(qbp, csp) bind(C)
    use, intrinsic :: iso_c_binding, only: c_int, c_ptr
    implicit none
    type(c_ptr), value :: qbp
    type(c_ptr), value :: csp
    integer(c_int) :: dmeasure_qubit
  end function dmeasure_qubit

  !cq_status dmeasure_qureg(qubit * qr, const size_t NQUBITS, cstate * cr);
  function dmeasure_qureg(qr, NQUBITS, cr) bind(C)
    use, intrinsic :: iso_c_binding, only: c_int, c_ptr, c_short, c_size_t
    implicit none
    integer(c_size_t), value :: NQUBITS
    type(c_ptr), value :: qr
    integer(c_short) :: cr(0:NQUBITS)
    integer(c_int) :: dmeasure_qureg
  end function dmeasure_qureg

  !cq_status dmeasure(qubit * qr, const size_t NQUBITS, size_t const * const TARGETS, 
  !  const size_t NTARGETS, cstate * cr);
  function dmeasure(qr, NQUBITS, TARGETS, NTARGETS, cr) bind(C)
    use, intrinsic :: iso_c_binding, only: c_int, c_ptr, c_size_t, c_short
    implicit none
    integer(c_size_t), value :: NQUBITS
    integer(c_size_t), value :: NTARGETS
    integer(c_short) :: cr(0:NTARGETS)
    integer(c_size_t) :: TARGETS(0:NTARGETS)
    type(c_ptr), value :: qr
    integer(c_int) :: dmeasure
  end function dmeasure

  !cq_status measure_qubit(qubit * qbp, cstate * csp);
  function measure_qubit(qbp, csp) bind(C)
    use, intrinsic :: iso_c_binding, only: c_int, c_ptr
    implicit none
    type(c_ptr), value :: qbp
    type(c_ptr), value :: csp
    integer(c_int) :: measure_qubit
  end function measure_qubit

  !cq_status measure_qureg(qubit * qr, const size_t NQUBITS, cstate * cr);
  function measure_qureg(qr, NQUBITS, cr) bind(C)
    use, intrinsic :: iso_c_binding, only: c_int, c_ptr, c_size_t, c_short
    implicit none
    type(c_ptr), value :: qr
    integer(c_size_t), value :: NQUBITS
    integer(c_short) :: cr(0:NQUBITS)
    integer(c_int) :: measure_qureg
  end function measure_qureg

  !cq_status measure(qubit * qr, const size_t NQUBITS, size_t const * const TARGETS, 
  !  const size_t NTARGETS, cstate * cr);
  function measure(qr, NQUBITS, TARGETS, NTARGETS, cr) bind(C)
    use, intrinsic :: iso_c_binding, only: c_int, c_ptr, c_size_t, c_short
    implicit none
    integer(c_size_t), value :: NQUBITS
    integer(c_size_t), value :: NTARGETS
    integer(c_short) :: cr(0:NTARGETS)
    integer(c_size_t) :: TARGETS(0:NTARGETS)
    type(c_ptr), value :: qr
    integer(c_int) :: measure
  end function measure

! -------------------------------- QASM Gates --------------------------------

  !cq_status unitary(qubit * qh, const double THETA, const double PHI, 
  !  const double LAMBDA);
  function unitary(qh, THETA, PHI, LAMBDA) bind(C)
    use, intrinsic :: iso_c_binding, only: c_int, c_ptr, c_double
    implicit none
    type(c_ptr), value :: qh
    real(c_double), value :: THETA
    real(c_double), value :: PHI
    real(c_double), value :: LAMBDA
    integer(c_int) :: unitary
  end function unitary

  !cq_status gphase(qubit * qh, const double THETA);
  function gphase(qh, THETA) bind(C)
    use, intrinsic :: iso_c_binding, only: c_int, c_ptr, c_double
    implicit none
    type(c_ptr), value :: qh
    real(c_double), value  :: THETA
    integer(c_int) :: gphase
  end function gphase

  !cq_status phase(qubit * qh, const double THETA);
  function phase(qh, THETA) bind(C)
    use, intrinsic :: iso_c_binding, only: c_int, c_ptr, c_double
    implicit none
    type(c_ptr), value :: qh
    real(c_double), value :: THETA
    integer(c_int) :: phase
  end function phase

  !cq_status paulix(qubit * qh);
  function paulix(qh) bind(C)
    use, intrinsic :: iso_c_binding, only: c_int, c_ptr
    implicit none
    type(c_ptr), value :: qh
    integer(c_int) :: paulix
  end function paulix

  !cq_status pauliy(qubit * qh);
  function pauliy(qh) bind(C)
    use, intrinsic :: iso_c_binding, only: c_int, c_ptr
    implicit none
    type(c_ptr), value :: qh
    integer(c_int) :: pauliy
  end function pauliy


  !cq_status pauliz(qubit * qh);
  function pauliz(qh) bind(C)
    use, intrinsic :: iso_c_binding, only: c_int, c_ptr
    implicit none
    type(c_ptr), value :: qh
    integer(c_int) :: pauliz
  end function pauliz

  !cq_status hadamard(qubit * qh);
  function hadamard(qh) bind(C)
    use, intrinsic :: iso_c_binding, only: c_int, c_ptr
    implicit none
    type(c_ptr), value :: qh
    integer(c_int) :: hadamard
  end function hadamard

  !cq_status sqrtz(qubit * qh);
  function sqrtz(qh) bind(C)
    use, intrinsic :: iso_c_binding, only: c_int, c_ptr
    implicit none
    type(c_ptr), value :: qh
    integer(c_int) :: sqrtz
  end function sqrtz

  !cq_status sqrtzhc(qubit * qh);
  function sqrtzhc(qh) bind(C)
    use, intrinsic :: iso_c_binding, only: c_int, c_ptr
    implicit none
    type(c_ptr), value :: qh
    integer(c_int) :: sqrtzhc
  end function sqrtzhc


  !cq_status sqrts(qubit * qh);
  function sqrts(qh) bind(C)
    use, intrinsic :: iso_c_binding, only: c_int, c_ptr
    implicit none
    type(c_ptr), value :: qh
    integer(c_int) :: sqrts
  end function sqrts


  !cq_status sqrtshc(qubit * qh);
  function sqrtshc(qh) bind(C)
    use, intrinsic :: iso_c_binding, only: c_int, c_ptr
    implicit none
    type(c_ptr), value :: qh
    integer(c_int) :: sqrtshc
  end function sqrtshc


  !cq_status sqrtx(qubit * qh);
  function sqrtx(qh) bind(C)
    use, intrinsic :: iso_c_binding, only: c_int, c_ptr
    implicit none
    type(c_ptr), value :: qh
    integer(c_int) :: sqrtx
  end function sqrtx

  !cq_status rotx(qubit * qh, const double THETA);
  function rotx(qh, THETA) bind(C)
    use, intrinsic :: iso_c_binding, only: c_int, c_ptr, c_double
    implicit none
    type(c_ptr), value :: qh
    real(c_double), value :: THETA
    integer(c_int) :: rotx
  end function rotx

  !cq_status roty(qubit * qh, const double THETA);
  function roty(qh, THETA) bind(C)
    use, intrinsic :: iso_c_binding, only: c_int, c_ptr, c_double
    implicit none
    type(c_ptr), value :: qh
    real(c_double), value :: THETA
    integer(c_int) :: roty
  end function roty
 
  !cq_status rotz(qubit * qh, const double THETA);
  function rotz(qh, THETA) bind(C)
    use, intrinsic :: iso_c_binding, only: c_int, c_ptr, c_double
    implicit none
    type(c_ptr), value :: qh
    real(c_double), value :: THETA
    integer(c_int) :: rotz
  end function rotz

  !cq_status cpaulix(qubit * ctrl, qubit * target);
  function cpaulix(ctrl, qtarget) bind(C)
    use, intrinsic :: iso_c_binding, only: c_int, c_ptr
    implicit none
    type(c_ptr), value :: ctrl
    type(c_ptr), value :: qtarget
    integer(c_int) :: cpaulix
  end function cpaulix

  !cq_status cpauliy(qubit * ctrl, qubit * target);
  function cpauliy(ctrl, qtarget) bind(C)
    use, intrinsic :: iso_c_binding, only: c_int, c_ptr
    implicit none
    type(c_ptr), value :: ctrl
    type(c_ptr), value :: qtarget
    integer(c_int) :: cpauliy
  end function cpauliy


  !cq_status cpauliz(qubit * ctrl, qubit * target);
  function cpauliz(ctrl, qtarget) bind(C)
    use, intrinsic :: iso_c_binding, only: c_int, c_ptr
    implicit none
    type(c_ptr), value :: ctrl
    type(c_ptr), value :: qtarget
    integer(c_int) :: cpauliz
  end function cpauliz

  !cq_status cphase(qubit * ctrl, qubit * target, const double THETA);
  function cphase(ctrl, qtarget, THETA) bind(C)
    use, intrinsic :: iso_c_binding, only: c_int, c_ptr, c_double
    implicit none
    type(c_ptr), value :: ctrl
    type(c_ptr), value :: qtarget
    real(c_double), value :: THETA
    integer(c_int) :: cphase
  end function cphase

  !cq_status crotx(qubit * ctrl, qubit *  target, const double THETA);
  function crotx(ctrl, qtarget, THETA) bind(C)
    use, intrinsic :: iso_c_binding, only: c_int, c_ptr, c_double
    implicit none
    type(c_ptr), value :: ctrl
    type(c_ptr), value :: qtarget
    real(c_double), value :: THETA
    integer(c_int) :: crotx
  end function crotx

  !cq_status croty(qubit * ctrl, qubit *  target, const double THETA);
  function croty(ctrl, qtarget, THETA) bind(C)
    use, intrinsic :: iso_c_binding, only: c_int, c_ptr, c_double
    implicit none
    type(c_ptr), value :: ctrl
    type(c_ptr), value :: qtarget
    real(c_double), value :: THETA
    integer(c_int) :: croty
  end function croty

  !cq_status crotz(qubit * ctrl, qubit *  target, const double THETA);
  function crotz(ctrl, qtarget, THETA) bind(C)
    use, intrinsic :: iso_c_binding, only: c_int, c_ptr, c_double
    implicit none
    type(c_ptr), value :: ctrl
    type(c_ptr), value :: qtarget
    real(c_double), value :: THETA
    integer(c_int) :: crotz
  end function crotz

  !cq_status chadamard(qubit * ctrl, qubit * target);
  function chadamard(ctrl, qtarget) bind(C)
    use, intrinsic :: iso_c_binding, only: c_int, c_ptr
    implicit none
    type(c_ptr), value :: ctrl
    type(c_ptr), value :: qtarget
    integer(c_int) :: chadamard
  end function chadamard


  !cq_status cunitary(qubit * ctrl, qubit * target, const double THETA, 
  !  const double PHI, const double LAMBDA);
  function cunitary(ctrl, qtarget, THETA, PHI, LAMBDA) bind(C)
    use, intrinsic :: iso_c_binding, only: c_int, c_ptr, c_double
    implicit none
    type(c_ptr), value :: ctrl
    type(c_ptr), value :: qtarget
    real(c_double), value :: THETA
    real(c_double), value :: PHI
    real(c_double), value :: LAMBDA
    integer(c_int) :: cunitary
  end function cunitary

  !cq_status swap(qubit * a, qubit * b);
  function swap(a, b) bind(C)
    use, intrinsic :: iso_c_binding, only: c_int, c_ptr
    implicit none
    type(c_ptr), value :: a
    type(c_ptr), value :: b
    integer(c_int) :: swap
  end function swap

  !cq_status ccpaulix(qubit * ctrl_a, qubit * ctrl_b, qubit * target);
  function ccpaulix(ctrl_a, ctrl_b, qtarget) bind(C)
    use, intrinsic :: iso_c_binding, only: c_int, c_ptr
    implicit none
    type(c_ptr), value :: ctrl_a
    type(c_ptr), value :: ctrl_b
    type(c_ptr), value :: qtarget
    integer(c_int) :: ccpaulix
  end function ccpaulix

  !cq_status cswap(qubit * ctrl, qubit * a, qubit * b);
  function cswap(ctrl, a, b) bind(C)
    use, intrinsic :: iso_c_binding, only: c_int, c_ptr
    implicit none
    type(c_ptr), value :: ctrl
    type(c_ptr), value :: a
    type(c_ptr), value :: b
    integer(c_int) :: cswap
  end function cswap

end interface

end module c_device_interface
