{ lib, stdenv, fetchFromGitHub, cmake, ninja, pkg-config, openmp, mpi, numactl
, shared ? true, enableOpenmp ? true, enableMpi ? true
, enableSubcomm ? enableMpi, enableNuma ? stdenv.hostPlatform.isLinux
}:
assert lib.assertMsg (!enableSubcomm || enableMpi) "QuEST subcommunicators require MPI.";
assert lib.assertMsg (!enableNuma || stdenv.hostPlatform.isLinux) "QuEST NUMA requires Linux.";
let
  options = {
    BUILD_SHARED_LIBS = shared;
    CMAKE_POSITION_INDEPENDENT_CODE = true;
    QUEST_FLOAT_PRECISION = 2;
    QUEST_ENABLE_OMP = enableOpenmp;
    QUEST_ENABLE_MPI = enableMpi;
    QUEST_ENABLE_SUBCOMM = enableSubcomm;
    QUEST_ENABLE_NUMA = enableNuma;
    QUEST_ENABLE_BMI2 = false;
    QUEST_ENABLE_CUDA = false;
    QUEST_ENABLE_CUQUANTUM = false;
    QUEST_ENABLE_HIP = false;
    QUEST_ENABLE_ADIOS2 = false;
    QUEST_DOWNLOAD_ADIOS2 = false;
    QUEST_TESTS_DOWNLOAD_CATCH2 = false;
    QUEST_BUILD_MIN_EXAMPLE = false;
    QUEST_BUILD_EXAMPLES = false;
    QUEST_BUILD_TESTS = false;
    QUEST_INSTALL_BINARIES = false;
    QUEST_ENABLE_INSTALL = true;
    QUEST_ENABLE_PACKAGING = false;
  };
in stdenv.mkDerivation {
  pname = "quest";
  version = "4.3.0-cmake-packaging";
  src = fetchFromGitHub {
    owner = "eessmann";
    repo = "QuEST";
    rev = "503552065045eaf89baba85e6cd6aad728525554";
    hash = "sha256-q2ZvDK57q5kjOxOYFivUexH8ARN8Q5zCf5HVUG+FwSY=";
  };
  nativeBuildInputs = [ cmake ninja pkg-config ];
  buildInputs = lib.optionals (enableOpenmp && stdenv.cc.isClang) [ openmp ]
    ++ lib.optional enableMpi mpi
    ++ lib.optional (enableNuma && enableOpenmp) numactl;
  cmakeFlags = lib.mapAttrsToList (name: value:
    if builtins.isBool value then lib.cmakeBool name value
    else "-D${name}=${toString value}") options;
  passthru = { inherit options shared enableOpenmp enableMpi enableSubcomm enableNuma; };
  meta = { license = lib.licenses.mit; platforms = lib.platforms.unix; };
}
