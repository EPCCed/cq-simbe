#define CQ_REGISTER_KERNEL(fname, reg)          \
  status = cq_register_fort_kernel(fname, reg); \
  if (status == 0) return

#define CQ_PROG_BEGIN() \
  if (.NOT.cq_is_device()) then
#define CQ_PROG_END() end if
