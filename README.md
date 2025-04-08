# CQ Simulated Backend

:warning: :construction: **CQ-SimBE is UNDER CONSTRUCTION** :construction: :warning:

A [CQ](https://github.com/EPCCed/cq-spec) compliant quantum computing library built on QuEST.

Designed to simulate tightly coupled offload to a quantum accelerator.

## Dependencies

- C99
- pthreads
- [QuEST](https://github.com/QuEST-Kit/QuEST) v4
- [Unity](https://github.com/ThrowTheSwitch/Unity) testing framework

## Implementation Roadmap

- [x] Minimum working example.
  - Synchronous QFT offload with threads is up and running!
- [x] Write tests for MWE.
- [ ] Implement the rest of the spec for the offload-to-thread version.
  - And don't forget to add tests...
- [ ] Implement Fortran (2003?) interface.
- [ ] Implement offloading with MPI (using a device _process_ instead of a device _thread_).