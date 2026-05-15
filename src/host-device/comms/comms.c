#include "src/host-device/comms.h"

#include "comms_core.h"

#include "quest/include/environment.h"

#include <stdio.h>

int initialise_device(const unsigned int VERBOSITY) {
  init_device_controls(VERBOSITY);
  unsigned int verbosity = VERBOSITY;
  host_send_ctrl_op(CQ_CTRL_INIT, &verbosity);
  host_wait_all_ops();
  return 0;
}

size_t host_send_ctrl_op(const enum ctrl_code OP, void* ctrl_params) {
  switch (OP) {
    case CQ_CTRL_SYNC_EXEC: {
      return comms_exec_sync(ctrl_params);
      break;
    }
    case CQ_CTRL_WAIT_EXEC: {
      return comms_exec_wait(ctrl_params);
      break;
    }
    case CQ_CTRL_ABORT: {
      comms_exec_halt(ctrl_params);
      return 0;
      break;
    }
    default: {
      break;
    }
  }
  return insert_op(OP, ctrl_params);
}

size_t host_wait_all_ops(void) {
  return device_wait_all_ops();
}

void host_device_sync_comms(void) {}

int finalise_device(const unsigned int VERBOSITY) {
  // Politely wait for the device to finish its current business
  // otherwise setting dev_ctrl.run_device = false might break
  // some stuff, and this should only be called in cq_finalise()
  host_wait_all_ops();

  if (VERBOSITY > 0) {
    printf("Finalising device.\n");
  }

  stop_device();
  unsigned int verbosity = VERBOSITY;
  host_send_ctrl_op(CQ_CTRL_FINALISE, &verbosity);
  finalise_device_controls(VERBOSITY);

  return 0;
}

bool is_device(void) {
  return false;
}

void init_quest_env(void) {
  initQuESTEnv();
}
