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

  module procedure cq_alloc_qureg !(qrp, N) result(status)
    status = alloc_qureg(qrp%this, N)
  end procedure cq_alloc_qureg

  module procedure cq_free_qureg !(qrp) result(status)
    status = free_qureg(qrp%this)
  end procedure cq_free_qureg

  module procedure cq_init_creg !(LENGTH, INIT_VAL, cr) result(status)
    call init_creg(LENGTH, INIT_VAL, cr)
  end procedure cq_init_creg

  module procedure cq_register_qkern !(kernel)
    status = register_qkern(kernel%target)
  end procedure cq_register_qkern

  module procedure cq_sm_qrun !(kernel, qrp, NQUBITS, crp, NMEASURE, NSHOTS) bind(C) result(status)
    status = sm_qrun(kernel%target, qrp%this, NQUBITS, crp, NMEASURE, NSHOTS)
  end procedure

end submodule host_ops


