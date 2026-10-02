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
They provide CMake 4.4.2, Ninja, MPICH, QuEST, and Unity. macOS uses LLVM 22
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

The presets honor `CC`, `CXX`, and `CMAKE_PREFIX_PATH`; CMake never downloads
dependencies. Use a fresh build directory when changing compilers or QuEST
variants. `nix develop .#clang` selects Clang; Linux also offers `.#gcc`.
`nix develop .#static-quest` provides PIC-enabled static QuEST, and
`nix develop .#non-mpi` disables MPI and SUBCOMM. Use separate build paths for
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

Tests additionally require `BUILD_TESTING=ON` (CTest's default). The old
`ENABLE_TESTING` option remains a deprecated fallback when
`CQ_SIMBE_BUILD_TESTS` has not been set. An explicit project option wins.
`CMAKE_INSTALL_LIBDIR`, `CMAKE_INSTALL_INCLUDEDIR`, and other GNUInstallDirs
settings control installation destinations.

## Downstream CMake projects

The installed package preserves `<cq.h>` and the individual public header
names. It exports one target:

```cmake
cmake_minimum_required(VERSION 4.0)
project(MyApplication LANGUAGES C)
find_package(CQ-SIMBE 0.1 CONFIG REQUIRED)
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
headers do not acquire QuEST or MPI includes. Enabling CXX for dependency
resolution does not change the public API into a C++ API.

## MPI lifecycle

`cq_init` and `cq_finalise` keep their existing signatures. In an MPI-enabled
build, all ranks in `MPI_COMM_WORLD` must call them collectively and in the same
order. The world size must be a power of two. QuEST receives a duplicate of
that communicator through its SUBCOMM API; there is no public communicator
selection API in cq-simbe.

If MPI is uninitialized, cq-simbe initializes it on its device worker with
`MPI_THREAD_MULTIPLE` and finalizes it on that worker after QuEST shutdown.
If a rank cannot create its worker, that rank participates in collective
startup rejection on the caller, initializing and finalizing MPI there if
necessary so that the other ranks can exit cleanly.
If the application already owns MPI, it must have requested and received
`MPI_THREAD_MULTIPLE`; cq-simbe leaves MPI active after `cq_finalise`. This
permits MPI calls alongside asynchronous quantum work without relying on an
external serialization convention. Lifecycle calls themselves must not race
with other cq-simbe lifecycle calls or application MPI finalization.

Finalized MPI, insufficient thread support, an unsupported rank count, and an
independently initialized QuEST environment are rejected. A failed CQ startup
does not mark its environment initialized. QuEST's own fatal validation
behavior remains in effect. Failed startup after CQ initializes MPI also
finalizes that MPI instance, so retry requires a new process.
Neither QuEST nor CQ-owned MPI can be reinitialized
after finalization; isolate lifecycle experiments in separate processes.

## Package validation

Run this inside the development environment:

```sh
cmake -DSOURCE_DIR="$PWD" -DBINARY_DIR="$PWD/out/package-check" \
  -P tests/packaging/check-package.cmake
```

The check builds shared and static cq-simbe, installs and relocates each,
compiles every public header, runs a separate consumer with kernel execution,
and checks embedding without Unity. Run it again in the `static-quest` shell
with a separate `BINARY_DIR` to cover static QuEST. CTest also includes isolated
MPI ownership, thread-support, startup-cleanup, and one-/two-rank execution
checks. GitHub Actions runs the pinned Linux GCC/Clang environments; successful
local macOS checks do not imply Linux or external HPC validation.

## Implementation Roadmap

- [x] Minimum working example.
  - Synchronous QFT offload with threads is up and running!
- [x] Write tests for MWE.
- [ ] Implement the rest of the spec for the offload-to-thread version.
  - And don't forget to add tests...
- [ ] Implement Fortran (2003?) interface.
- [ ] Implement offloading with MPI (using a device _process_ instead of a device _thread_).
