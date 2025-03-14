#ifndef CQ_OP_CODES_H
#define CQ_OP_CODES_H

enum ctrl_code {
  INIT,
  ABORT,
  FINALISE,
  ALLOC,
  DEALLOC,
  RUN_QKERNEL,
  RUN_PQKERNEL
};

#endif