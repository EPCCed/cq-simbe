module consumer_kernel_module
  use cq
#include "cqf.h"
  implicit none
contains
  function kernel(nqubits, qr, nmeasure, cr, reg) bind(C) result(status)
    integer(c_size_t), value :: nqubits, nmeasure
    type(qubit), value :: qr
    integer(c_short), intent(inout) :: cr(0:nmeasure-1)
    type(qkern_map), value :: reg
    integer(c_int) :: status
    CQ_REGISTER_KERNEL("consumer_kernel", reg)
    status = cq_set_qureg(qr, 0_c_size_t, nqubits)
    if (status /= 0) return
    status = cq_measure_qureg(qr, nmeasure, cr)
  end function kernel
end module consumer_kernel_module

program consumer
  use cq
  use consumer_kernel_module
#include "cqf.h"
  implicit none
  type(qubit) :: qr
  integer(c_short) :: result(1)
  integer :: status
  status = cq_init(0)
  if (status /= 0) stop 1
  status = cq_register_qkern(kernel)
  if (status /= 0) stop 2
  CQ_PROG_BEGIN()
    status = cq_alloc_qureg(qr, 1_c_size_t)
    if (status /= 0) stop 3
    result = -1
    status = cq_s_qrun(kernel, qr, 1_c_size_t, result, 1_c_size_t)
    if (status /= 0 .or. result(1) /= 0) stop 4
    status = cq_free_qureg(qr)
    if (status /= 0) stop 5
  CQ_PROG_END()
  status = cq_finalise(0)
  if (status /= 0) stop 6
end program consumer
