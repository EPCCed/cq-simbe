// I think that the comms.h should stay the same (and ideally
// the rest of the code) and if we compile with MPI build, then
// we link against this file. Otherwise against comms.c

#include "mpi_comms.h"
#include <mpi_proto.h>
#include <stddef.h>
#include <stdio.h>
#include "datatypes.h"
#include "src/device/control.h"
#include "src/host-device/comms.h"
#include "src/host/opcodes.h"

#include <mpi.h>
#include <stdlib.h>
#include <string.h>

static MPI_Comm cq_mpi_comm = MPI_COMM_WORLD;

struct cq_mpi_env {
  int rank;
};

static struct cq_mpi_env mpi_env;

const char* get_comm_source() {
  if (mpi_env.rank == CQ_MPI_HOST_RANK) {
    return "Host:\t\t";
  } else if (mpi_env.rank == CQ_MPI_DEVICE_RANK) {
    return "Device:\t\t";
  } else {
    return "";
  }
}
void print_params_header(struct ctrl_params_header header) {
  //  if (mpi_env.rank == CQ_MPI_HOST_RANK) {
  //    printf("Host [header]: ");
  //  } else {
  //    printf("Device [header]: ");
  //  }
  switch (header.type) {
    case PARAMS_UINT_T: {
      printf("%s [header]: type: UINT, ", get_comm_source());
      break;
    }
    case PARAMS_ALLOC_T: {
      //      printf("type: ALLOC, ");
      printf("%s [header]: type: ALLOC, ", get_comm_source());
      break;
    }
    case PARAMS_EXEC_T: {
      //     printf("type: EXEC, ");
      printf("%s [header]: type: EXEC, ", get_comm_source());
      break;
    }
    default: {
      break;
    }
  }
  printf(" %d\n", header.params_msg_size);
}

void print_alloc_params(void* params) {
  device_alloc_params* alloc_params = (device_alloc_params*)params;

  printf("%s alloc params: NQUBITS: %zu, qreg_idx: %zu, STATUS: %d\n",
         get_comm_source(), alloc_params->NQUBITS, alloc_params->qregistry_idx,
         alloc_params->status);
}

void print_op(const enum ctrl_code OP) {
  switch (OP) {
    case CQ_CTRL_IDLE: {
      printf("CQ_CTRL_IDLE");
      break;
    }
    case CQ_CTRL_ALLOC: {
      printf("CQ_CTRL_ALLOC");
      break;
    }
    case CQ_CTRL_DEALLOC: {
      printf("CQ_CTRL_DEALLOC");
      break;
    }
    case CQ_CTRL_INIT: {
      printf("CQ_CTRL_INIT");
      break;
    }
    case CQ_CTRL_FINALISE: {
      printf("CQ_CTRL_FINALISE");
      break;
    }
    case CQ_CTRL_RUN_QKERNEL: {
      printf("CQ_CTRL_RUN_QKERNEL");
      break;
    }
    case CQ_CTRL_RUN_PQKERNEL: {
      printf("CQ_CTRL_RUN_PQKERNEL");
      break;
    }
    case CQ_CTRL_TEST: {
      printf("CQ_CTRL_TEST");
      break;
    }
    case CQ_CTRL_ABORT: {
      printf("CQ_CTRL_ABORT");
      break;
    }
    default: {
      break;
    }
  }
  printf("\n");
}

enum ctrl_params_datatype op_type_to_params_type(enum ctrl_code op_type)

{
  switch (op_type) {
    case CQ_CTRL_ALLOC:
      return PARAMS_ALLOC_T;
    case CQ_CTRL_DEALLOC:
      return PARAMS_ALLOC_T;
    case CQ_CTRL_INIT:
      return PARAMS_UINT_T;
    case CQ_CTRL_FINALISE:
      return PARAMS_UINT_T;
    case CQ_CTRL_RUN_QKERNEL:
      return PARAMS_EXEC_T;
    case CQ_CTRL_RUN_PQKERNEL:
      return PARAMS_EXEC_T;
    default: {
      printf("Unhandled op type. Returning PARAMS_UINT_T. Expect segfault.\n");
      return PARAMS_UINT_T;
    };
  }
}

size_t static get_params_size(enum ctrl_params_datatype type) {
  switch (type) {
    case PARAMS_UINT_T: {
      return sizeof(unsigned int);
      break;
    }
    case PARAMS_ALLOC_T: {
      return sizeof(device_alloc_params);
      break;
    }
    case PARAMS_EXEC_T: {
      return 0;
      break;
    }
    default: {
      return 0;
      break;
    }
  }
}

void params_deep_copy(enum ctrl_params_datatype params_type,
                      void* src,
                      void** dest) {
  const size_t params_size = get_params_size(params_type);
  *dest = malloc(params_size);
  memcpy(*dest, src, params_size);
}

// void send_uint_params(void* params) {
// TODO: do the destination parameter
void send_uint_params(struct device_ctrl_params ctrl_params, int dest) {
  MPI_Datatype uint_type =
      sizeof(unsigned int) == 4 ? MPI_UINT32_T : MPI_UINT64_T;

  struct ctrl_params_header msg_header = {
      .type = ctrl_params.type, .params_msg_size = sizeof(unsigned int)};
  int header_size = sizeof(struct ctrl_params_header);

  MPI_Ssend(&msg_header, header_size, MPI_BYTE, dest, CQ_HOST_DEVICE_MPI_TAG,
            cq_mpi_comm);
  MPI_Ssend(ctrl_params.data, 1, uint_type, dest, CQ_HOST_DEVICE_MPI_TAG,
            cq_mpi_comm);
}

void recv_uint_params(void** params, int params_size, int src) {
  printf("%s Attempting to recieve verbosity param...\n", get_comm_source());
  if (params == NULL) {
    printf("From recv_uint_params: params is NULL.\n");
    exit(-2);
  }

  // free previous, now unused resources
  if (*params != NULL) {
    printf("From recv_uint_params: *params is NULL.\n");
    exit(-2);

    free(params);
    *params = NULL;
  }

  MPI_Status status;
  *params = malloc(params_size);
  if (*params == NULL) {
    printf("From recv_uint_params: malloc failed.\n");
    exit(-2);
  }

  MPI_Recv(*params, params_size, MPI_BYTE, src, CQ_HOST_DEVICE_MPI_TAG,
           cq_mpi_comm, &status);
}

// void send_alloc_params(void* params) {
void send_alloc_params(struct device_ctrl_params ctrl_params, int dest) {
  // TODO: use memcpy because no pointers in alloc params
  //
  // pack device_alloc_params
  //  device_alloc_params* alloc_params =
  //  (device_alloc_params*)ctrl_params.data; int max_buffer_size;
  //  MPI_Pack_size(1, MPI_UINT64_T, cq_mpi_comm, &max_buffer_size);  // NQUBITS
  //  int member_size;
  //  MPI_Pack_size(1, MPI_UINT64_T, cq_mpi_comm, &member_size);  //
  //  qregistry_idx max_buffer_size += member_size; MPI_Pack_size(1, MPI_INT,
  //  cq_mpi_comm, &member_size);  // status max_buffer_size += member_size;
  //
  //  char* send_buffer = malloc(max_buffer_size);
  //  if (send_buffer == NULL) {
  //    printf("Failed to allocate buffer for sending device allocation
  //    params.\n"); exit(-1);
  //  }
  //  int position;
  //  MPI_Pack(&alloc_params->NQUBITS, 1, MPI_UINT64_T, send_buffer,
  //           max_buffer_size, &position, cq_mpi_comm);
  //  MPI_Pack(&alloc_params->qregistry_idx, 1, MPI_UINT64_T, send_buffer,
  //           max_buffer_size, &position, cq_mpi_comm);
  //  MPI_Pack(&alloc_params->status, 1, MPI_INT, send_buffer, max_buffer_size,
  //           &position, cq_mpi_comm);

  // all nicely packed
  //  struct ctrl_params_header msg_header = {.type = ctrl_params.type,
  //                                          .params_msg_size = position};

  printf("%s Attempting to send device_alloc_params...\n", get_comm_source());
  size_t buffer_size = sizeof(device_alloc_params);
  void* send_buffer = malloc(buffer_size);
  memcpy(send_buffer, ctrl_params.data, buffer_size);
  struct ctrl_params_header msg_header = {.type = ctrl_params.type,
                                          .params_msg_size = buffer_size};

  int header_size = sizeof(struct ctrl_params_header);

  MPI_Ssend(&msg_header, header_size, MPI_BYTE, dest, CQ_HOST_DEVICE_MPI_TAG,
            cq_mpi_comm);
  MPI_Ssend(send_buffer, buffer_size, MPI_BYTE, dest, CQ_HOST_DEVICE_MPI_TAG,
            cq_mpi_comm);

  //  MPI_Ssend(send_buffer, position, MPI_PACKED, device_rank,
  //  CQ_HOST_DEVICE_MPI_TAG,
  //            cq_mpi_comm);

  free(send_buffer);
  printf("%s Sent device_alloc_params...\n", get_comm_source());
}

void recv_alloc_params(void** params, int params_size, int src) {
  printf("%s Attempting to recieve device_alloc_params...\n",
         get_comm_source());
  if (params == NULL) {
    printf("From recv_alloc_params: params is NULL.\n");
    exit(-2);
  }

  // free previous, now unused resources
  if (*params != NULL) {
    printf("From recv_alloc_params: *params is NULL.\n");
    exit(-2);
    free(params);
    *params = NULL;
  }

  MPI_Status status;
  *params = malloc(params_size);
  if (*params == NULL) {
    printf("From recv_alloc_params: malloc failed.\n");
    exit(-2);
  }

  MPI_Recv(*params, params_size, MPI_BYTE, src, CQ_HOST_DEVICE_MPI_TAG,
           cq_mpi_comm, &status);
  printf("%s Recieved device_alloc_params...\n", get_comm_source());
}

void send_exec_params(struct device_ctrl_params ctrl_params, int dest) {}
void recv_exec_params(void** params, int params_size, int src) {}

// void comm_ctrl_params(struct ctrl_params_header msg_header,
//                       void* params,
//                       param_packer_fn packer) {
//   if (mpi_env.rank == CQ_MPI_HOST_RANK) {
//     send_ctrl_params(msg_header, params, packer);
//   } else {
//     recv_ctrl_params();
//   }
// }

// void send_ctrl_params(struct ctrl_params_header msg_header,
//                       void* params,
//                       param_packer_fn packer) {
//   int device_rank = 1;
//   size_t header_size = sizeof(struct ctrl_params_header);
//   MPI_Ssend(&msg_header, header_size, MPI_BYTE, device_rank,
//             CQ_HOST_DEVICE_MPI_TAG, cq_mpi_comm);
//   print_params_header(msg_header);

void send_ctrl_params(const struct device_ctrl_params* ctrl_params, int dest) {
  switch (ctrl_params->type) {
    case PARAMS_UINT_T: {
      send_uint_params(*ctrl_params, dest);
      break;
    }
    case PARAMS_ALLOC_T: {
      send_alloc_params(*ctrl_params, dest);
      break;
    }
    case PARAMS_EXEC_T: {
      send_exec_params(*ctrl_params, dest);
      break;
    }
    default: {
      break;
    }
  }
}

void recv_ctrl_params(void** params, int src) {
  struct ctrl_params_header msg_header;
  size_t header_size = sizeof(struct ctrl_params_header);
  MPI_Status status;
  MPI_Recv(&msg_header, header_size, MPI_BYTE, src, CQ_HOST_DEVICE_MPI_TAG,
           cq_mpi_comm, &status);
  print_params_header(msg_header);
  switch (msg_header.type) {
    case PARAMS_UINT_T: {
      recv_uint_params(params, msg_header.params_msg_size, src);
      break;
    }
    case PARAMS_ALLOC_T: {
      recv_alloc_params(params, msg_header.params_msg_size, src);
      break;
    }
    case PARAMS_EXEC_T: {
      recv_exec_params(params, msg_header.params_msg_size, src);
      break;
    }
    default: {
      break;
    }
  }
}

int mpi_initialise_device(const unsigned int VERBOSITY) {
  printf("Initialising MPI.\n");
  MPI_Init(NULL, NULL);
  printf("Initialised MPI.\n");
  MPI_Comm_rank(MPI_COMM_WORLD, &mpi_env.rank);

  if (mpi_env.rank != CQ_MPI_HOST_RANK) {
    if (VERBOSITY > 0) {
      printf("Initialising device.\n");
    }
    pthread_mutex_init(&dev_ctrl.device_lock, NULL);
    pthread_cond_init(&dev_ctrl.cond_device_busy, NULL);
    pthread_cond_init(&dev_ctrl.cond_queue_empty, NULL);
    pthread_cond_init(&dev_ctrl.cond_queue_full, NULL);

    dev_ctrl.run_device = true;
    dev_ctrl.device_busy = true;
    dev_ctrl.num_ops = 0;
    dev_ctrl.next_op_in = 0;
    dev_ctrl.next_op_out = 0;

    for (size_t i = 0; i < __CQ_DEVICE_QUEUE_SIZE__; ++i) {
      dev_ctrl.op_buffer[i] = CQ_CTRL_IDLE;
      dev_ctrl.op_params_buffer[i] = NULL;
    }

    pthread_create(&dev_ctrl.device_thread, NULL, &mpi_device_control_thread,
                   NULL);
  }

  unsigned int verbosity = VERBOSITY;
  // struct ctrl_params_header msg_header = {
  //     .type = PARAMS_UINT_T, .params_msg_size = sizeof(unsigned int)};
  // comm_ctrl_params(msg_header, &verbosity, &pack_uint_params);

  struct device_ctrl_params params = {.type = PARAMS_UINT_T,
                                      .data = &verbosity};
  mpi_host_comm_ctrl_op(CQ_CTRL_INIT, &params);
  printf("Finished: comm_ctrl_op\n");
  mpi_host_wait_all_ops();
  printf("Finished: wait_all_ops\n");

  return 0;
}

void mpi_host_comm_ctrl_op(const enum ctrl_code OP,
                           struct device_ctrl_params* ctrl_params) {
  if (mpi_env.rank == CQ_MPI_HOST_RANK) {
    mpi_host_send_ctrl_op(OP, ctrl_params);
    printf("%s [comm_ctrl_op]: finished sending ctrl op\n", get_comm_source());
  } else {
    mpi_host_recv_ctrl_op();
    printf("%s [comm_ctrl_op]: finished recv ctrl op\n", get_comm_source());
  }
}

// TODO: Need to know the size of ctrl_params
// maybe instead have struct like:
// struct CtrlParams {
//  void* data;
//  int size;
// }
// and wrap it whenever there is call to host_send_ctrl_op
size_t mpi_host_send_ctrl_op(const enum ctrl_code OP,
                             struct device_ctrl_params* ctrl_params) {
  const int device_rank = CQ_MPI_DEVICE_RANK;
  // TODO: merge sends?
  MPI_Ssend(&OP, 1, MPI_INT, device_rank, CQ_HOST_DEVICE_MPI_TAG, cq_mpi_comm);
  printf("%s [send_ctrl_op]: Sent ", get_comm_source());
  print_op(OP);
  send_ctrl_params(ctrl_params, device_rank);

  size_t num_ops = 0;
  MPI_Status status;
  MPI_Recv(&num_ops, 1, MPI_UINT64_T, device_rank, CQ_HOST_DEVICE_MPI_TAG,
           cq_mpi_comm, &status);
  printf("%s recv num ops: %zu\n", get_comm_source(), num_ops);

  // alloc and dealloc should be blocking
  if (OP == CQ_CTRL_ALLOC || OP == CQ_CTRL_DEALLOC) {
    void* params_buffer = NULL;
    recv_ctrl_params(&params_buffer, device_rank);
    memcpy(ctrl_params->data, params_buffer, sizeof(device_alloc_params));
    printf("Performed deep copy\n");
    print_alloc_params(ctrl_params->data);
  }

  // rest handled by executor

  return num_ops;
}

// TODO: rename -- it is device comms thread that runs this
size_t mpi_host_recv_ctrl_op() {
  const int host_rank = CQ_MPI_HOST_RANK;
  pthread_mutex_lock(&dev_ctrl.device_lock);

  while (dev_ctrl.num_ops >= __CQ_DEVICE_QUEUE_SIZE__) {
    // the control queue is full!
    // we'll wait for it to not be full
    pthread_cond_wait(&dev_ctrl.cond_queue_full, &dev_ctrl.device_lock);
  }

  MPI_Status status;
  MPI_Recv(&dev_ctrl.op_buffer[dev_ctrl.next_op_in], 1, MPI_INT, host_rank,
           CQ_HOST_DEVICE_MPI_TAG, cq_mpi_comm, &status);
  printf("%s [recv_ctrl_op]: Recieved ", get_comm_source());
  print_op(dev_ctrl.op_buffer[dev_ctrl.next_op_in]);

  recv_ctrl_params(&dev_ctrl.op_params_buffer[dev_ctrl.next_op_in], host_rank);
  if (dev_ctrl.op_buffer[dev_ctrl.next_op_in] == CQ_CTRL_ALLOC ||
      dev_ctrl.op_buffer[dev_ctrl.next_op_in] == CQ_CTRL_DEALLOC) {
    print_alloc_params(dev_ctrl.op_params_buffer[dev_ctrl.next_op_in]);
  } else {
    printf("%s verbosity params: VERBOSITY: %d\n", get_comm_source(),
           *(unsigned int*)dev_ctrl.op_params_buffer[dev_ctrl.next_op_in]);
  }

  ++dev_ctrl.num_ops;

  // It's a ring buffer!
  // advance next_op_in then mod out buffer size
  ++dev_ctrl.next_op_in;
  dev_ctrl.next_op_in %= __CQ_DEVICE_QUEUE_SIZE__;

  MPI_Ssend(&dev_ctrl.num_ops, 1, MPI_UINT64_T, host_rank,
            CQ_HOST_DEVICE_MPI_TAG, cq_mpi_comm);
  printf("%s [recv_ctrl_op]: sent num ops: %zu\n", get_comm_source(),
         dev_ctrl.num_ops);

  pthread_cond_signal(&dev_ctrl.cond_queue_empty);
  pthread_mutex_unlock(&dev_ctrl.device_lock);

  return dev_ctrl.num_ops;
}

size_t mpi_host_wait_all_ops(void) {
  if (mpi_env.rank == CQ_MPI_HOST_RANK) {
    const int device_rank = CQ_MPI_DEVICE_RANK;
    size_t num_ops = 0;
    MPI_Status status;
    printf("%s [wait_all_ops]: got here\n", get_comm_source());
    // rather than waiting for num ops, wait for the ctrl params and update
    // recv_ctrl_params(void** params, int src)
    MPI_Recv(&num_ops, 1, MPI_UINT64_T, device_rank, CQ_HOST_DEVICE_MPI_TAG,
             cq_mpi_comm, &status);
    return num_ops;

  } else {
    const int host_rank = CQ_MPI_HOST_RANK;
    pthread_mutex_lock(&dev_ctrl.device_lock);
    int foo = 0;
    while (dev_ctrl.num_ops > 0 || dev_ctrl.device_busy) {
      printf("%s [wait_all_ops][loop]: got here %d\n", get_comm_source(), foo);
      ++foo;
      pthread_cond_wait(&dev_ctrl.cond_device_busy, &dev_ctrl.device_lock);
    }
    printf("%s [wait_all_ops]: got here\n", get_comm_source());
    // rather than sending num ops, device_control_thread sends updated params?
    // but also maybe I don't want MPI inside device_control_thread
    MPI_Ssend(&dev_ctrl.num_ops, 1, MPI_UINT64_T, host_rank,
              CQ_HOST_DEVICE_MPI_TAG, cq_mpi_comm);

    pthread_mutex_unlock(&dev_ctrl.device_lock);
    return dev_ctrl.num_ops;
  }
}

void* mpi_device_control_thread(void*) {
  enum ctrl_code current_op = CQ_CTRL_IDLE;
  void* current_op_params = NULL;

  // run_device set to FALSE at cq_finalise
  while (dev_ctrl.run_device) {
    pthread_mutex_lock(&dev_ctrl.device_lock);

    while (dev_ctrl.num_ops <= 0) {
      // wait for a new op to be posted
      dev_ctrl.device_busy = false;
      pthread_cond_signal(&dev_ctrl.cond_device_busy);
      pthread_cond_wait(&dev_ctrl.cond_queue_empty, &dev_ctrl.device_lock);
    }

    dev_ctrl.device_busy = true;

    // take the next op and params out of the dev_ctrl buffer, and then tidy
    // up the dev_ctrl buffer
    current_op = dev_ctrl.op_buffer[dev_ctrl.next_op_out];

    // TODO: Can solve NOTE 4 (see below) by doing deep copy here...
    // and freeing op_params_buffer earlier
    // current_op_params = dev_ctrl.op_params_buffer[dev_ctrl.next_op_out];

    params_deep_copy(op_type_to_params_type(current_op),
                     dev_ctrl.op_params_buffer[dev_ctrl.next_op_out],
                     &current_op_params);

    if (current_op_params == NULL) {
      printf("Params are null!\n");
    }
    print_alloc_params(current_op_params);
    dev_ctrl.op_buffer[dev_ctrl.next_op_out] = CQ_CTRL_IDLE;
    // release resources allocated in specialisation of recv_ctrl_params
    // Alternativally comment this out and handle freeing in those
    // specialisations -- see commented out first few lines in recv_uint_params.
    free(dev_ctrl.op_params_buffer[dev_ctrl.next_op_out]);  // allocated in
                                                            // recv_ctrl_params
    dev_ctrl.op_params_buffer[dev_ctrl.next_op_out] = NULL;
    // NOTE:1 -- 01.05.2026: went with freeing in recievers solution
    // NOTE:2 -- 01.05.2026: since we have ring buffer, resources will be freed
    // on snd pass
    // NOTE:3 -- 05.05.2026:  actually reverted and went with freeing resources
    // here as sometimes I want update params in the reciever (see end of this
    // function)

    // const size_t last_op_idx = dev_ctrl.next_op_out;

    //  decrease the number of queued operations and advance next_op_out
    --dev_ctrl.num_ops;
    ++dev_ctrl.next_op_out;
    dev_ctrl.next_op_out %= __CQ_DEVICE_QUEUE_SIZE__;

    // signal that the queue is no longer full and then relinquish mutex
    pthread_cond_signal(&dev_ctrl.cond_queue_full);
    pthread_mutex_unlock(&dev_ctrl.device_lock);

    control_registry[current_op](current_op_params);

    // TODO: send updated ctrl_params to host
    // also consider if this is the right place for the send...
    // NOTE:4 -- 05.05.2026: This is quite nasty because:
    // a) I don't wont to block thread for control_registry duration
    // b) but I need to free the resources after and lock the dev_ctrl
    // c) but meanwhile, other threads can modify dev_ctrl...
    if (op_type_to_params_type(current_op) != PARAMS_UINT_T) {
      struct device_ctrl_params params_to_sync = {
          .type = op_type_to_params_type(current_op),
          .data = current_op_params};
      print_alloc_params(params_to_sync.data);
      const int host_rank = CQ_MPI_HOST_RANK;
      send_ctrl_params(&params_to_sync, host_rank);
    }
    free(current_op_params);  // allocated in params_deep_copy();
  }

  pthread_mutex_unlock(&dev_ctrl.device_lock);

  return NULL;
}

int mpi_finalise_device(const unsigned int VERBOSITY) {
  // Politely wait for the device to finish its current business
  // otherwise setting dev_ctrl.run_device = false might break
  // some stuff, and this should only be called in cq_finalise()
  mpi_host_wait_all_ops();

  if (mpi_env.rank != CQ_MPI_HOST_RANK) {
    if (VERBOSITY > 0) {
      printf("Finalising device.\n");
    }

    pthread_mutex_lock(&dev_ctrl.device_lock);
    dev_ctrl.run_device = false;
    pthread_mutex_unlock(&dev_ctrl.device_lock);
  }

  unsigned int verbosity = VERBOSITY;
  struct device_ctrl_params params = {.type = PARAMS_UINT_T,
                                      .data = &verbosity};
  mpi_host_comm_ctrl_op(CQ_CTRL_FINALISE, &params);

  if (mpi_env.rank != CQ_MPI_HOST_RANK) {
    pthread_join(dev_ctrl.device_thread, NULL);

    dev_ctrl.device_busy = false;

    pthread_cond_destroy(&dev_ctrl.cond_device_busy);
    pthread_cond_destroy(&dev_ctrl.cond_queue_empty);
    pthread_cond_destroy(&dev_ctrl.cond_queue_full);
    pthread_mutex_destroy(&dev_ctrl.device_lock);
  }

  MPI_Finalize();
  return 0;
}
// struct DeviceLink dev_ctrl;
//
// int mpi_initialise_device(const unsigned int VERBOSITY) {
//   if (VERBOSITY > 0) {
//     printf("Initialising device.\n");
//   }
//
//   int nprocs;
//   int world_rank;
//   MPI_Comm comm_split;
//
//   MPI_Init(NULL, NULL);
//
//   MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
//   MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
//   // int in_quest_comm = world_rank != 0;
//   // MPI_Comm_split(MPI_COMM_WORLD, I_AM_QUANTUM, world_rank, &comm_split);
//
//   dev_ctrl.run_device = true;
//   dev_ctrl.device_busy = true;
//   dev_ctrl.num_ops = 0;
//   dev_ctrl.next_op_in = 0;
//   dev_ctrl.next_op_out = 0;
//
//   for (size_t i = 0; i < __CQ_DEVICE_QUEUE_SIZE__; ++i) {
//     dev_ctrl.op_buffer[i] = CQ_CTRL_IDLE;
//     dev_ctrl.op_params_buffer[i] = NULL;
//   }
//
//   unsigned int verbosity = VERBOSITY;
//   mpi_host_send_ctrl_op(CQ_CTRL_INIT, &verbosity);
//   mpi_host_wait_all_ops();
//
//   return 0;
// }
//
// size_t mpi_host_send_ctrl_op(const enum ctrl_code OP, void* ctrl_params) {
//   // 1. need to request from device queue size...
//   // 2. send OP for device op_buffer
//   int rank;
//   MPI_Comm_rank(MPI_COMM_WORLD, &rank);
//   if (rank == 0) {
//     MPI_Status status;
//     int device_rank = 1;
//
//     while (dev_ctrl.num_ops >= __CQ_DEVICE_QUEUE_SIZE__) {
//       MPI_Recv(&dev_ctrl.num_ops, 1, MPI_UINT64_T, device_rank, 0,
//                MPI_COMM_WORLD, &status);
//     }
//     MPI_Ssend(&OP, 1, MPI_INT, device_rank, 0, MPI_COMM_WORLD);
//     // TODO: this needs solution
//     // MPI_Ssend(ctrl_params, ctrl_params_extent, MPI_CHAR, device_rank, 0,
//     // MPI_COMM_WORLD);
//
//     ++dev_ctrl.num_ops;
//     ++dev_ctrl.next_op_in;
//     dev_ctrl.next_op_in %= __CQ_DEVICE_QUEUE_SIZE__;
//
//     // NOTE: matched by Recv on lines 120 and 122
//     MPI_Ssend(&dev_ctrl.num_ops, 1, MPI_UINT64_T, device_rank, 0,
//               MPI_COMM_WORLD);
//     MPI_Ssend(&dev_ctrl.next_op_in, 1, MPI_UINT64_T, device_rank, 0,
//               MPI_COMM_WORLD);
//
//     MPI_Recv(&dev_ctrl.num_ops, 1, MPI_UINT64_T, device_rank, 0,
//     MPI_COMM_WORLD,
//              &status);
//     MPI_Recv(&dev_ctrl.next_op_in, 1, MPI_UINT64_T, device_rank, 0,
//              MPI_COMM_WORLD, &status);
//
//   } else {
//     // NOTE: okay, I think it's simply wrong place for this code
//     // actually we leave host rank as it is and this code goes to the device
//     // code or something
//     //    int host_rank = 0;
//     //    MPI_Status status;
//     //
//     //    // NOTE: if the device queue is full this will deadlock
//     //    // instead we have to call host_send_ctrl_op with a modification
//     //    MPI_Ssend(&dev_ctrl.num_ops, 1, MPI_UINT64_T, host_rank, 0,
//     //    MPI_COMM_WORLD);
//     //
//     //    MPI_Recv(&dev_ctrl.op_buffer[dev_ctrl.next_op_in], 1, MPI_UINT64_T,
//     //             host_rank, 0, MPI_COMM_WORLD, &status);
//     //
//     //    // TODO: this is tricky and needs solving
//     //    // MPI_Recv(&dev_ctrl.op_params_buffer[dev_ctrl.next_op_in],
//     //    // ctrl_params_extent, MPI_CHAR,
//     //    //         host_rank, 0, MPI_COMM_WORLD, &status);
//     //
//     //    ++dev_ctrl.num_ops;
//     //    ++dev_ctrl.next_op_in;
//     //    dev_ctrl.next_op_in %= __CQ_DEVICE_QUEUE_SIZE__;
//     //    MPI_Ssend(&dev_ctrl.num_ops, 1, MPI_UINT64_T, host_rank, 0,
//     //    MPI_COMM_WORLD); MPI_Ssend(&dev_ctrl.next_op_in, 1, MPI_UINT64_T,
//     //    host_rank, 0,
//     //              MPI_COMM_WORLD);
//     //  }
//   }
//   return dev_ctrl.num_ops;
// }
//
// void* mpi_device_control_thread(void* par) {
//   enum ctrl_code current_op = CQ_CTRL_IDLE;
//   void* current_op_params = NULL;
//   int host_rank = 0;
//   MPI_Status status;
//
//   // run_device set to FALSE at cq_finalise
//   while (dev_ctrl.run_device) {
//     // pthread_mutex_lock(&dev_ctrl.device_lock);
//
//     while (dev_ctrl.num_ops <= 0) {
//       // wait for a new op to be posted
//       dev_ctrl.device_busy = false;
//       MPI_Recv(&dev_ctrl.num_ops, 1, MPI_UINT64_T, host_rank, 0,
//       MPI_COMM_WORLD,
//                &status);
//       MPI_Recv(&dev_ctrl.next_op_in, 1, MPI_UINT64_T, host_rank, 0,
//                MPI_COMM_WORLD, &status);
//
//       pthread_cond_signal(&dev_ctrl.cond_device_busy);
//       pthread_cond_wait(&dev_ctrl.cond_queue_empty, &dev_ctrl.device_lock);
//     }
//     MPI_Recv(&dev_ctrl.op_buffer[dev_ctrl.next_op_in], 1, MPI_UINT64_T,
//              host_rank, 0, MPI_COMM_WORLD, &status);
//
//     dev_ctrl.device_busy = true;
//
//     // take the next op and params out of the dev_ctrl buffer, and then tidy
//     // up the dev_ctrl buffer
//     current_op = dev_ctrl.op_buffer[dev_ctrl.next_op_out];
//     current_op_params = dev_ctrl.op_params_buffer[dev_ctrl.next_op_out];
//     dev_ctrl.op_buffer[dev_ctrl.next_op_out] = CQ_CTRL_IDLE;
//     dev_ctrl.op_params_buffer[dev_ctrl.next_op_out] = NULL;
//
//     // decrease the number of queued operations and advance next_op_out
//     --dev_ctrl.num_ops;
//     ++dev_ctrl.next_op_out;
//     dev_ctrl.next_op_out %= __CQ_DEVICE_QUEUE_SIZE__;
//
//     // signal that the queue is no longer full and then relinquish mutex
//     pthread_cond_signal(&dev_ctrl.cond_queue_full);
//     pthread_mutex_unlock(&dev_ctrl.device_lock);
//
//     control_registry[current_op](current_op_params);
//   }
//
//   pthread_mutex_unlock(&dev_ctrl.device_lock);
//
//   return NULL;
// }
