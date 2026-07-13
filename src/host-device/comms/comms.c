#include "src/host-device/comms.h"

#include "comms_core.h"

#include "quest/include/environment.h"

#include <stdio.h>

int initialise_device(const unsigned int VERBOSITY) {
  return serial_initialise_device(VERBOSITY);
}

size_t host_send_ctrl_op(const enum ctrl_code OP, void * ctrl_params) {
  return serial_host_send_ctrl_op(OP, ctrl_params);
}

size_t host_wait_all_ops(void) {
  return serial_host_wait_all_ops();
}

void host_device_sync_comms(void) {
  serial_host_device_sync_comms();
}

int finalise_device(const unsigned int VERBOSITY) {
  return serial_finalise_device(VERBOSITY);
}

bool is_device(void) {
  return false;
}

void init_quest_env(void) {
  initQuESTEnv();
}
