/* Exercise the actual queue/host implementation, replacing only device
   dispatch and pthread creation primitives to inject bounded failures. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include "env.h"
#include "src/host-device/comms.h"

#define CHECK(condition) do { if (!(condition)) { \
  fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #condition); return 1; \
} } while (0)
static int fail_step, step, live_mutexes, live_conditions, live_threads;
static cq_status init_result = CQ_ERROR, final_result = CQ_SUCCESS;
static int shim_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr) {
  if (++step == fail_step) return EAGAIN;
  int result = pthread_mutex_init(mutex, attr);
  if (!result) live_mutexes++;
  return result;
}
static int shim_mutex_destroy(pthread_mutex_t *mutex) {
  int result = pthread_mutex_destroy(mutex);
  if (!result) live_mutexes--;
  return result;
}
static int shim_cond_init(pthread_cond_t *cond, const pthread_condattr_t *attr) {
  if (++step == fail_step) return EAGAIN;
  int result = pthread_cond_init(cond, attr);
  if (!result) live_conditions++;
  return result;
}
static int shim_cond_destroy(pthread_cond_t *cond) {
  int result = pthread_cond_destroy(cond);
  if (!result) live_conditions--;
  return result;
}
static int shim_create(pthread_t *thread, const pthread_attr_t *attr,
                       void *(*fn)(void *), void *arg) {
  if (++step == fail_step) return EAGAIN;
  int result = pthread_create(thread, attr, fn, arg);
  if (!result) live_threads++;
  return result;
}
static int shim_join(pthread_t thread, void **result) {
  int status = pthread_join(thread, result);
  if (!status) live_threads--;
  return status;
}
#define pthread_mutex_init shim_mutex_init
#define pthread_mutex_destroy shim_mutex_destroy
#define pthread_cond_init shim_cond_init
#define pthread_cond_destroy shim_cond_destroy
#define pthread_create shim_create
#define pthread_join shim_join
#include "src/host-device/comms.c"
#include "src/host/env.c"

int isQuESTEnvInit(void) { return 0; }
cq_status cq_mpi_preflight(int ready) { return ready ? CQ_SUCCESS : CQ_ERROR; }
void cq_mpi_cancel_preflight(void) {}
cq_status cq_mpi_prepare(int ready) { return ready ? CQ_SUCCESS : CQ_ERROR; }
static cq_status init(void *arg) { return init_result; }
static cq_status finish(void *arg) { return final_result; }
cq_status (*control_registry[8])(void *) = {init, NULL, finish};

int main(int argc, char **argv) {
  CHECK(argc == 2);
  fail_step = atoi(argv[1]);
  CHECK(cq_finalise(0) == CQ_WARNING);
  CHECK(cq_init(0) == CQ_ERROR);
  CHECK(!cq_env.initialised && !cq_env.finalised);
  CHECK(!dev_ctrl.worker_started && !dev_ctrl.run_device);
  CHECK(!live_mutexes && !live_conditions && !live_threads);
  CHECK(cq_finalise(0) == CQ_WARNING);
  CHECK(finalise_device(0) == CQ_WARNING);
  /* Every failure leaves the synchronization infrastructure reusable. */
  fail_step = 0;
  init_result = CQ_SUCCESS;
  CHECK(cq_init(0) == CQ_SUCCESS);
  CHECK(cq_env.initialised);
  final_result = CQ_ERROR;
  CHECK(cq_finalise(0) == CQ_ERROR);
  CHECK(!cq_env.finalised);
  CHECK(!live_mutexes && !live_conditions && !live_threads);
  CHECK(!dev_ctrl.worker_started && !dev_ctrl.run_device);
  return 0;
}
