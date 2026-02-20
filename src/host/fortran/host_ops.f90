submodule (cq) host_ops

use c_host_interface
use c_device_interface
implicit none

contains

! ------------------------------ HOST OPERATIONS ------------------------------
  module procedure cq_init !(VERBOSITY) result(status)
    status = fortran_cq_init(VERBOSITY)
  end procedure cq_init

  module procedure cq_finalise !(VERBOSITY) result(status)
    status = fortran_cq_finalise(VERBOSITY)
  end procedure cq_finalise

  module procedure cq_alloc_qubit !(qhp) result(status)
    status = alloc_qubit(qhp%this)
  end procedure cq_alloc_qubit

  module procedure cq_free_qubit !(qhp) result(status)
    status = free_qubit(qhp%this)
  end procedure cq_free_qubit

  module procedure cq_alloc_qureg !(qrp, N) result(status)
    status = alloc_qureg(qrp%this, N)
  end procedure cq_alloc_qureg

  module procedure cq_free_qureg !(qrp) result(status)
    status = free_qureg(qrp%this)
  end procedure cq_free_qureg

  module procedure cq_init_creg !(LENGTH, INIT_VAL, cr) result(status)
    call init_creg(LENGTH, INIT_VAL, cr)
  end procedure cq_init_creg

  module procedure cq_sync_qrun !(ehp) result(status)
    status = sync_qrun(ehp%this)
  end procedure cq_sync_qrun

  module procedure cq_wait_qrun !(ehp) result(status)
    status = wait_qrun(ehp%this)
  end procedure cq_wait_qrun

  module procedure cq_halt_qrun !(ehp) result(status)
    status = halt_qrun(ehp%this)
  end procedure cq_halt_qrun

  module procedure cq_create_exec_handle !(ehp) result(status)
    status = fort_create_exec_handle(ehp%this)
  end procedure cq_create_exec_handle

  module procedure cq_free_exec_handle !(ehp) result(status)
    status = fort_free_exec_handle(ehp%this)
  end procedure cq_free_exec_handle

end submodule host_ops


