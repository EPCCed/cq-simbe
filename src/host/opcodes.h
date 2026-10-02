#ifndef CQ_OP_CODES_H
#define CQ_OP_CODES_H

enum ctrl_code {
  CQ_CTRL_IDLE = -1,
  CQ_CTRL_INIT,
  CQ_CTRL_ABORT,
  CQ_CTRL_FINALISE,
  CQ_CTRL_ALLOC,
  CQ_CTRL_DEALLOC,
  CQ_CTRL_RUN_QKERNEL,
  CQ_CTRL_RUN_PQKERNEL,
  CQ_CTRL_TEST,
  CQ_CTRL_WAIT,  	// added in MPI extension
  CQ_CTRL_WAIT_EXEC,    // ditto
  CQ_CTRL_SYNC_EXEC,    // ditto
};

#endif
