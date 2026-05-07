#include "mpi_comms.h"

#include "src/host-device/comms.h"
#include "src/host/opcodes.h"

#include <mpi.h>
#include <mpi_proto.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

static MPI_Comm CQ_MPI_COMM = MPI_COMM_WORLD;

struct cq_mpi_env {
  int rank;
};

static struct cq_mpi_env mpi_env = {.rank = -1};

const char* get_comm_source() {
  if (mpi_env.rank == CQ_MPI_HOST_RANK) {
    return "Host:\t\t";
  } else if (mpi_env.rank == CQ_MPI_DEVICE_RANK) {
    return "Device:\t\t";
  } else {
    return "";
  }
}

int get_rank(void) {
  return mpi_env.rank;
}

const char* op_to_str(const enum ctrl_code OP) {
  switch (OP) {
    case CQ_CTRL_IDLE: {
      return "CQ_CTRL_IDLE";
      break;
    }
    case CQ_CTRL_ALLOC: {
      return "CQ_CTRL_ALLOC";
      break;
    }
    case CQ_CTRL_DEALLOC: {
      return "CQ_CTRL_DEALLOC";
      break;
    }
    case CQ_CTRL_INIT: {
      return "CQ_CTRL_INIT";
      break;
    }
    case CQ_CTRL_FINALISE: {
      return "CQ_CTRL_FINALISE";
      break;
    }
    case CQ_CTRL_WAIT: {
      return "CQ_CTRL_WAIT";
      break;
    }
    case CQ_CTRL_RUN_QKERNEL: {
      return "CQ_CTRL_RUN_QKERNEL";
      break;
    }
    case CQ_CTRL_RUN_PQKERNEL: {
      return "CQ_CTRL_RUN_PQKERNEL";
      break;
    }
    case CQ_CTRL_TEST: {
      return "CQ_CTRL_TEST";
      break;
    }
    case CQ_CTRL_ABORT: {
      return "CQ_CTRL_ABORT";
      break;
    }
    default: {
      break;
    }
  }
  return "";
}

void init_host_device_mpi(const unsigned int VERBOSITY) {
  if (VERBOSITY > 0) {
    printf("Initialising MPI.\n");
  }
  MPI_Init(NULL, NULL);
  MPI_Comm_rank(CQ_MPI_COMM, &mpi_env.rank);
  if (VERBOSITY > 0) {
    printf("Initialised MPI.\n");
  }

  if (mpi_env.rank == CQ_MPI_HOST_RANK) {
    // send INIT OP
    // const int device_rank = CQ_MPI_DEVICE_RANK;
    // const enum ctrl_code OP = CQ_CTRL_INIT;
    // printf("%s attempting to send OP\n", get_comm_source());
    // MPI_Ssend(&OP, 1, MPI_INT, device_rank, CQ_MPI_COMMS_TAG, CQ_MPI_COMM);
    // printf("%s sent OP\n", get_comm_source());
  } else {
    // recv Init Op
    // enum ctrl_code OP;
    // const int host_rank = CQ_MPI_HOST_RANK;
    // MPI_Status status;
    // MPI_Recv(&OP, 1, MPI_INT, host_rank, CQ_MPI_COMMS_TAG, CQ_MPI_COMM,
    //         &status);
    // setup device
    // initialise_device(VERBOSITY);
    // start listening   -- alternatively we can start pthread here
    dev_ctrl.run_device = true;
    printf("%s started listening...\n", get_comm_source());
    device_listen();
    printf("%s closing connection.\n", get_comm_source());
    finalise_host_device_mpi(VERBOSITY);
    //  if OP == Finalise
    //       break the loop
    //       and return from here
    //  then pass through all host ops
    //  and in cq-finalise cleanup resources
  }
}

void finalise_host_device_mpi(const unsigned int VERBOSITY) {
  printf("%s Finalising MPI.\n", get_comm_source());
  MPI_Finalize();
  printf("Finalised MPI.\n");
}

void mpi_host_send_ctrl_op(const enum ctrl_code OP,
                           struct ctrl_params* params) {
  const int device_rank = CQ_MPI_DEVICE_RANK;
  printf("%s [send_ctrl_op]: sending %s...\n", get_comm_source(),
         op_to_str(OP));
  MPI_Ssend(&OP, 1, MPI_INT, device_rank, CQ_MPI_COMMS_TAG, CQ_MPI_COMM);
  printf("%s [send_ctrl_op]: sent %s\n", get_comm_source(), op_to_str(OP));
}

void mpi_host_wait_all_ops(void) {
  printf("%s [wait_all_ops]: waiting...\n", get_comm_source());
  mpi_host_send_ctrl_op(CQ_CTRL_WAIT, NULL);
  size_t num_ops;
  const int device_rank = CQ_MPI_DEVICE_RANK;
  MPI_Status status;
  MPI_Recv(&num_ops, 1, MPI_UINT64_T, device_rank, CQ_MPI_COMMS_TAG,
           CQ_MPI_COMM, &status);
  printf("%s [wait_all_ops]: all ops completed (num_ops: %zu)\n",
         get_comm_source(), num_ops);
}

// ----------------------------------------------------------------------------
// Device Comm Ops
// ----------------------------------------------------------------------------

void device_listen(void) {
  while (dev_ctrl.run_device) {
    enum ctrl_code OP;
    const int host_rank = CQ_MPI_HOST_RANK;
    MPI_Status status;
    printf("%s [device_listen]: recieving OP...\n", get_comm_source());
    MPI_Recv(&OP, 1, MPI_INT, host_rank, CQ_MPI_COMMS_TAG, CQ_MPI_COMM,
             &status);
    printf("%s [device_listen]: recieved %s\n", get_comm_source(),
           op_to_str(OP));

    // optionally get params and run op
    // by running op I mean modyfing the internal dev_ctrl fields
    // and then the worker thread handles the rest.
    device_dispatch_ctrl_op(OP);
    // if (OP == CQ_CTRL_FINALISE) {
    //   break;
    // }
  }
}

// aka host_send_ctrl_op from comm.c
void insert_op(const enum ctrl_code OP) {
  pthread_mutex_lock(&dev_ctrl.device_lock);

  while (dev_ctrl.num_ops >= __CQ_DEVICE_QUEUE_SIZE__) {
    // the control queue is full!
    // we'll wait for it to not be full
    pthread_cond_wait(&dev_ctrl.cond_queue_full, &dev_ctrl.device_lock);
  }

  unsigned int* ctrl_params = (unsigned int*)malloc(sizeof(unsigned int));
  *ctrl_params = 1;
  dev_ctrl.op_buffer[dev_ctrl.next_op_in] = OP;
  dev_ctrl.op_params_buffer[dev_ctrl.next_op_in] = ctrl_params;
  ++dev_ctrl.num_ops;

  // It's a ring buffer!
  // advance next_op_in then mod out buffer size
  ++dev_ctrl.next_op_in;
  dev_ctrl.next_op_in %= __CQ_DEVICE_QUEUE_SIZE__;

  pthread_cond_signal(&dev_ctrl.cond_queue_empty);
  pthread_mutex_unlock(&dev_ctrl.device_lock);
}

void device_dispatch_ctrl_op(const enum ctrl_code OP) {
  switch (OP) {
    case CQ_CTRL_INIT: {
      initialise_device(1);
      insert_op(OP);
      break;
    }
    case CQ_CTRL_FINALISE: {
      // insert_op(OP);
      device_wait_all_ops();
      finalise_device(1);
      break;
    }
    case CQ_CTRL_ALLOC: {
      break;
    }
    case CQ_CTRL_DEALLOC: {
      break;
    }
    case CQ_CTRL_RUN_QKERNEL: {
      break;
    }
    case CQ_CTRL_RUN_PQKERNEL: {
      break;
    }
    case CQ_CTRL_WAIT: {  // NOTE: added in MPI extension
      size_t num_ops = device_wait_all_ops();
      const int host_rank = CQ_MPI_HOST_RANK;
      printf("%s [dispatch]: sending num ops: %zu...\n", get_comm_source(),
             num_ops);
      MPI_Ssend(&num_ops, 1, MPI_UINT64_T, host_rank, CQ_MPI_COMMS_TAG,
                CQ_MPI_COMM);
      printf("%s [dispatch]: sent num ops.\n", get_comm_source());
      break;
    }
    case CQ_CTRL_ABORT: {
      break;
    }
    case CQ_CTRL_IDLE: {
      break;
    }
    case CQ_CTRL_TEST: {
      break;
    }
    default: {
      break;
    }
  }
}

size_t device_wait_all_ops(void) {
  pthread_mutex_lock(&dev_ctrl.device_lock);
  while (dev_ctrl.num_ops > 0 || dev_ctrl.device_busy) {
    pthread_cond_wait(&dev_ctrl.cond_device_busy, &dev_ctrl.device_lock);
  }

  pthread_mutex_unlock(&dev_ctrl.device_lock);
  return dev_ctrl.num_ops;
}
