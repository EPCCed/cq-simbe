#ifndef CQ_MPI_RUNTIME_H
#define CQ_MPI_RUNTIME_H

#include "datatypes.h"

/* Internal API: no MPI types escape into CQ's public headers. */
cq_status cq_mpi_preflight(int local_ready);
void cq_mpi_cancel_preflight(void);
cq_status cq_mpi_prepare(int local_ready);
cq_status cq_mpi_finish(void);
cq_status cq_mpi_agree_status(cq_status status, bool halt);

#endif
