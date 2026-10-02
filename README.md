# CQ Simulated Backend

:warning: :construction: **CQ-SimBE is UNDER CONSTRUCTION** :construction: :warning:

A [CQ](https://github.com/EPCCed/cq-spec) compliant quantum computing library built on QuEST.

Designed to simulate tightly coupled offload to a quantum accelerator.

## Dependencies

- CMake 4.0 or newer, and C/C++ compilers. The public API requires C99;
  QuEST supplies its own language requirements through its CMake target.
- POSIX threads and the platform math library.
- [QuEST 4.3](https://github.com/eessmann/QuEST/tree/cmake-packaging), using the
  corrected CMake package from the `cmake-packaging` branch. The development
  environment pins commit `503552065045eaf89baba85e6cd6aad728525554`.
- [Unity](https://github.com/ThrowTheSwitch/Unity) 2.6.1 with double-precision
  assertions enabled, only when building tests.

## Local development

The shared Nix environment supports Apple Silicon macOS and x86-64/AArch64
Linux. Enter it with either command:

```sh
devenv shell
# or
nix develop
```

Both entrypoints use the same locked nixpkgs and dependency definitions.
They provide CMake 4.4.2, Ninja, MPICH, QuEST, Unity, GFortran, and NLopt.
GFortran matches the compiler used for MPICH's Fortran bindings; NLopt is only
linked by the optional VQE example. macOS uses LLVM 22
with libc++; Linux defaults to GCC. QuEST defaults to shared, double precision,
OpenMP, MPI, and SUBCOMM, with NUMA enabled on Linux. GPU backends are disabled.
Unity is built with CMake so it exports `unity::framework` and propagates
`UNITY_INCLUDE_DOUBLE` consistently.

```sh
cmake --preset debug
cmake --build --preset debug
ctest --preset debug

cmake --preset release
cmake --build --preset release
ctest --preset release
```

The `full-debug` and `full-release` presets also enable the Fortran bindings
and NLopt VQE example.

The presets honor `CC`, `CXX`, `FC`, and `CMAKE_PREFIX_PATH`; CMake never downloads
dependencies. Use a fresh build directory when changing compilers or QuEST
variants. `nix develop .#clang` selects Clang; Linux also offers `.#gcc`.
`nix develop .#static-quest` provides PIC-enabled static QuEST, and
`nix develop .#non-mpi` disables MPI and SUBCOMM. The `serial-quest` shell
keeps MPICH available with serial QuEST: configure with
`-DCQ_SIMBE_ENABLE_MPI=ON` to validate transport MPI independently.
Use `-DCQ_SIMBE_ENABLE_MPI=OFF` with the default environment for explicit
thread transport. Use separate build paths for
these variants, for example `cmake --preset debug -B out/build/non-mpi-debug`.

Local devenv overrides belong in the ignored `devenv.local.nix`:

```nix
{
  cq-simbe.questOptions.shared = false;
  # cq-simbe.questOptions.enableMpi = false;
}
```

Commit both lockfiles when deliberately updating nixpkgs; the environment
rejects divergent revisions or content hashes. Shell entry does not update
the locks. IDEs should use the same shell's compiler and dependency environment.

## Building with external dependencies

An installed QuEST package can be supplied through `CMAKE_PREFIX_PATH` or
`QuEST_DIR`. A Git checkout of the fork is available with
`git clone --branch cmake-packaging git@github.com:eessmann/QuEST.git`;
check out the pinned commit above for the tested dependency source.

For MPI-enabled QuEST, build it with `QUEST_ENABLE_MPI=ON` and
`QUEST_ENABLE_SUBCOMM=ON`. Use the same MPI implementation and compatible
C++ runtime throughout. Static QuEST must be built with
`CMAKE_POSITION_INDEPENDENT_CODE=ON` when linking shared cq-simbe.

```sh
cmake -S . -B out/build/external -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_PREFIX_PATH=/path/to/dependencies \
  -DCQ_SIMBE_BUILD_TESTS=OFF
cmake --build out/build/external
cmake --install out/build/external --prefix /path/to/cq-simbe
```

| Option | Standalone default | Embedded default |
| --- | --- | --- |
| `BUILD_SHARED_LIBS` | `ON` | Inherits the parent's setting; otherwise `ON` |
| `CQ_SIMBE_BUILD_TESTS` | `ON` | `OFF` |
| `CQ_SIMBE_BUILD_EXAMPLES` | `ON` | `OFF` |
| `CQ_SIMBE_ENABLE_INSTALL` | `ON` | `OFF` |
| `CQ_SIMBE_ENABLE_MPI` | Detected QuEST MPI support | Detected QuEST MPI support |
| `CQ_SIMBE_BUILD_FORTRAN` | `OFF` | `OFF` |
| `CQ_SIMBE_BUILD_VQE` | `OFF` | `OFF` |

Tests additionally require `BUILD_TESTING=ON` (CTest's default). The old
`ENABLE_TESTING`, `BUILD_EXAMPLES`, `BUILD_WITH_MPI`,
`BUILD_FORTRAN_INTERFACE`, and `FORCE_BUILD_VQE` options remain deprecated
fallbacks for their modern counterparts when those are unset. An explicit
modern option wins. QuEST capabilities are always detected from the installed
package; the old `QUEST_BUILT_WITH_MPI` toggle is ignored.
`CMAKE_INSTALL_LIBDIR`, `CMAKE_INSTALL_INCLUDEDIR`, and other GNUInstallDirs
settings control installation destinations.

## Downstream CMake projects

The installed package preserves `<cq.h>` and the individual public header
names. Its core target is:

```cmake
cmake_minimum_required(VERSION 4.0)
project(MyApplication LANGUAGES C)
find_package(CQ-SIMBE 0.2 CONFIG REQUIRED)
add_executable(application main.c)
target_link_libraries(application PRIVATE CQ-SIMBE::cq-simbe)
```

Point `CMAKE_PREFIX_PATH` at the installation. Shared-library consumers need
the runtime dependency libraries but do not need QuEST's development package.
For a static cq-simbe installation, use `LANGUAGES C CXX` and make the QuEST
package and its dependencies discoverable as well. The package config resolves
those dependencies; consumers need no manual thread, MPI, QuEST, or math links.
Package version compatibility is restricted to the same minor release while
the library is at version 0.x.

Embedding with `add_subdirectory` provides the same target and leaves tests,
examples, and installation disabled unless explicitly requested. Public C
headers do not require QuEST headers. MPI transport publishes
`CQ_WITH_MPI_COMMS=1` and `MPI::MPI_C`, because the custom initialization API
uses `MPI_Comm`. Enabling CXX for dependency resolution does not change the
public API into a C++ API.

The optional Fortran package retains module `cq` and header `cqf.h`:

```cmake
project(MyFortranApplication LANGUAGES C Fortran) # add CXX for static CQ
find_package(CQ-SIMBE 0.2 CONFIG REQUIRED COMPONENTS Fortran)
add_executable(application main.f90)
set_target_properties(application PROPERTIES Fortran_PREPROCESS ON)
target_link_libraries(application PRIVATE CQ-SIMBE::fortran)
```

Enable Fortran before requesting this component. Installed module artifacts
are qualified by compiler identity and version; the component requires the
same compiler identity/version as the producer. Core consumers never load
the Fortran target or need a Fortran compiler.

Define registered `bind(C)` kernels as module procedures. Internally contained
procedures can require executable trampolines and fail with GFortran on Darwin;
the Fortran examples show the portable module form.

Version 0.2 adopts the MPI branch's API/ABI: kernels receive `NMEASURE`,
parameterized run calls include `KERNPAR_SIZE`, and executor handles have
additional transport state. Rebuild consumers and update kernel signatures
when moving from 0.1.

## MPI lifecycle and execution

MPI transport is the default when the installed QuEST supports MPI. Rank 0
of the CQ communicator is the host; other ranks run device workers. A singleton
falls back to a local worker. With distributed QuEST, supported sizes are
`1 + 2^k` (2, 3, 5, 9, ...) plus singleton; with serial QuEST, use one or two
ranks. The device group must have a power-of-two size.

With MPI transport, `cq_init`, `cq_init_custom_mpi_comm`, kernel registration,
and `cq_finalise` are collective on the participating communicator and must occur in the same
order on all its ranks. Custom initialization requires a valid caller-owned
intracommunicator. Other world ranks need not participate. CQ duplicates the
application communicator and separates command traffic, worker control, and
QuEST collectives; QuEST manages its own subcommunicator duplicate.

```c
if (cq_init(0) != CQ_SUCCESS) return 1;
if (register_qkern(my_kernel) != CQ_SUCCESS) return 1;
CQ_PROG_BEGIN()
  /* Allocate registers, submit executions, and collect results on the host. */
CQ_PROG_END()
return cq_finalise(0) != CQ_SUCCESS;
```

Use one host control thread for CQ calls. With MPI transport, registration is collective,
including when a device reaches registration later than the host. Register
before submission. Keep executor handles, quantum registers, parameter data,
and classical result buffers alive until the execution is collected with
`wait_qrun` (or `halt_qrun`), or drained by `cq_finalise`.
`sync_qrun` only polls progress and does not release the executor slot.
Parameter buffers sent to devices must contain values, not pointers to host memory. Completed executor
slots remain reserved until results are collected; submission returns
`CQ_ERROR` when all slots are occupied. Finalization collects outstanding
results and shuts down both worker and communication threads.

CQ requires `MPI_THREAD_MULTIPLE`, allowing caller MPI operations alongside
asynchronous worker activity. In offload mode, CQ initializes and finalizes
owned MPI on the calling thread. Caller-owned MPI remains active after CQ
shutdown. The explicit thread backend (`CQ_SIMBE_ENABLE_MPI=OFF`) retains
worker-owned MPI initialization/finalization when QuEST itself uses MPI; all
world ranks then act as hosts and the world size must be a power of two.
Thread-backend applications must register matching kernels and submit work in
the same order on every rank; registration itself remains local in this mode.

Finalized MPI, insufficient thread support, unsupported rank counts, and
independently initialized QuEST environments are rejected. Capabilities and
startup readiness are agreed across ranks before QuEST initialization. Failed
startup cleans up without marking CQ initialized. QuEST fatal validation
errors retain their native behavior. CQ-owned MPI cannot be reinitialized after
finalization; isolate lifecycle experiments in separate processes. Lifecycle
calls must not race with application MPI finalization.

## Package validation

Run this inside the development environment:

```sh
cmake -DSOURCE_DIR="$PWD" -DBINARY_DIR="$PWD/out/package-check" \
  -DCQ_SIMBE_BUILD_FORTRAN=ON \
  -P tests/packaging/check-package.cmake
```

The check builds shared and static cq-simbe, installs and relocates each,
compiles every public header, runs a separate consumer with kernel execution,
and checks embedding without Unity. Run it again in the `static-quest` shell
with a separate `BINARY_DIR` to cover static QuEST. CTest also includes isolated
MPI ownership, custom communicators, thread support, startup cleanup,
registration, executor capacity, distributed halt, and one-/two-/three-/five-rank
execution checks. Internal simulator tests run with thread transport; use both
Debug and Release for each backend. The `full-*` presets exercise Fortran and
VQE, while VQE kernel validation remains independent of NLopt. GitHub Actions
runs the pinned Linux GCC/Clang environments; successful local macOS checks do not imply Linux or external HPC validation.

## Implementation Roadmap

- [x] Minimum working example.
  - Synchronous QFT offload with threads is up and running!
- [x] Write tests for MWE.
- [ ] Implement the rest of the spec for the offload-to-thread version.
  - And don't forget to add tests...
- [x] Implement the optional Fortran interface.
- [x] Implement offloading with MPI device processes.
