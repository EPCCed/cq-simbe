#define CQ_REGISTER_KERNEL(fname, reg)                                         \
    status = cq_register_fort_kernel(fname, reg);                              \
    if (status == 0) return
