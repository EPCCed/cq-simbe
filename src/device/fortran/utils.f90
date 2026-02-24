module device_utils
use cq
use, intrinsic :: iso_c_binding
implicit none

contains
  function get_qubit_at(qh, qubit_idx) result(new_ptr)
    implicit none
    type(qubit), value :: qh
    integer(c_intptr_t), value :: qubit_idx
    type(c_ptr) :: new_ptr
    type(qubit_t) :: qubit_size

    ! validate representation of c_intptr_t and c_ptr
    integer(c_intptr_t) :: unused_i
    type(c_ptr) :: unused_p
    integer, parameter :: pointer_sized = &
    merge(c_intptr_t, -1, storage_size(unused_i) == storage_size(unused_p))
    integer(pointer_sized) :: ip
    ! end
    ip = transfer(qh%this, ip)
    ip = ip + (c_sizeof(qubit_size) * qubit_idx)
    new_ptr = transfer(ip, new_ptr)
  end function get_qubit_at

end module device_utils
