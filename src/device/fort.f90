module device_ops
implicit none
use c_device_interface
use c_host_interface

!type, bind(C) :: qubit
!  integer(c_size_t) :: registry_index
!  integer(c_size_t) :: offset
!  integer(c_size_t) :: N
!end type qubit

!type :: qubit_ptr
!  !! Wrapper type for implementing an array of pointers to qubit objects
!  type(qubit), pointer :: target
!end type qubit_ptr

interface 
  function cq_set_qureg(qr, STATE_IDX, NQUBITS)
    use, intrinsic :: iso_c_binding, only: c_size_t, c_ptr, c_int
    import :: qubit
    integer(kind=8), value :: NQUBITS
    type(qubit_ptr) :: qr(NQUBITS)
    integer(kind=8), value :: STATE_IDX
    integer :: cq_set_qureg
  end function

end interface

contains
  function cq_set_qureg(qr, STATE_IDX, NQUBITS)
    use, intrinsic :: iso_c_binding, only: c_size_t, c_ptr, c_int
    import :: qubit_ptr
    integer(kind=8), value :: NQUBITS
    type(qubit_ptr) :: qr(NQUBITS)
    integer(kind=8), value :: STATE_IDX
    integer :: cq_set_qureg

    cq_set_qureg = set_qureg(qr, STATE_IDX, NQUBITS)
  end function
 

end module
