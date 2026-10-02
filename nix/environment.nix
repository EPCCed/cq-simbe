# One compiler and dependency definition for devenv shell and nix develop.
{ pkgs, compiler ? "default", enableMpi ? true, questOptions ? {}, packageOverrides ? (_: {}) }:
assert import ./check-locks.nix;
let
  inherit (pkgs) lib;
  darwin = pkgs.stdenv.hostPlatform.isDarwin;
  stdenv = if darwin then pkgs.llvmPackages_22.libcxxStdenv
    else if compiler == "clang" then pkgs.llvmPackages_22.stdenv
    else pkgs.stdenv;
  base = final: {
    openmp = pkgs.llvmPackages_22.openmp;
    fortran = pkgs.gfortran;
    # NLopt exposes a C API to the VQE example. Keep nixpkgs' Clang build,
    # which avoids its documented GCC/FORTIFY test failure.
    nlopt = pkgs.nlopt;
    mpi = (pkgs.mpich.override { inherit stdenv; gfortran = final.fortran; }).overrideAttrs (old:
      lib.optionalAttrs darwin {
        # MPICH's Darwin sockets provider hangs during OFI finalization.
        configureFlags = old.configureFlags ++ [ "--with-device=ch4:ofi:tcp" ];
        # Versioned Mach-O names put the version before .dylib. Match the
        # concrete mpi/pmpi libraries so remove-references-to receives files.
        postFixup = builtins.replaceStrings
          [ "-name 'libmpi.dylib*'" ] [ "-name 'lib*mpi*.dylib'" ] old.postFixup;
      });
    unity = pkgs.callPackage ./unity.nix { inherit stdenv; };
    quest = pkgs.callPackage ./quest.nix ({
      inherit stdenv;
      inherit (final) openmp mpi;
    } // questOptions);
  };
  dependencies = lib.fix (final: let previous = base final; in previous // packageOverrides previous);
  cmakeDependencies = with dependencies; [ quest unity nlopt ]
    ++ lib.optional enableMpi dependencies.mpi
    ++ lib.optionals (dependencies.quest.enableOpenmp && stdenv.cc.isClang) [ dependencies.openmp ]
    ++ lib.optional (dependencies.quest.enableOpenmp && dependencies.quest.enableNuma) pkgs.numactl;
  packages = cmakeDependencies ++ [ dependencies.fortran pkgs.cmake pkgs.ninja pkgs.git pkgs.pkg-config ];
  env = {
    CC = "${stdenv.cc}/bin/${stdenv.cc.targetPrefix}cc";
    CXX = "${stdenv.cc}/bin/${stdenv.cc.targetPrefix}c++";
    FC = "${dependencies.fortran}/bin/gfortran";
    CMAKE_PREFIX_PATH = lib.makeSearchPath "" (map lib.getDev cmakeDependencies);
  } // lib.optionalAttrs (stdenv.hostPlatform.isLinux && stdenv.cc.isClang) (
    let
      # Clang reports host library directories even through its Nix wrapper.
      # CMake can replay these ahead of Nix's libc when GFortran links a C/C++
      # static library. Exclude only host paths, retaining the Nix runtimes.
      excluded = lib.concatStringsSep ";" [
        "/lib" "/lib32" "/lib64" "/usr/lib" "/usr/lib32" "/usr/lib64"
        "/lib/x86_64-linux-gnu" "/usr/lib/x86_64-linux-gnu"
        "/lib/aarch64-linux-gnu" "/usr/lib/aarch64-linux-gnu"
      ];
    in {
      CMAKE_C_IMPLICIT_LINK_DIRECTORIES_EXCLUDE = excluded;
      CMAKE_CXX_IMPLICIT_LINK_DIRECTORIES_EXCLUDE = excluded;
    });
in
assert lib.assertMsg (builtins.elem compiler [ "default" "gcc" "clang" ]) "Unknown compiler selection.";
assert lib.assertMsg (!darwin || compiler != "gcc") "The GCC shell is supported on Linux; Darwin uses LLVM 22/libc++.";
assert lib.assertMsg (enableMpi || !dependencies.quest.enableMpi) "An MPI-enabled QuEST requires MPI in the development environment.";
assert pkgs.cmake.version == "4.4.2";
{
  inherit stdenv dependencies cmakeDependencies packages env enableMpi;
  shell = (pkgs.mkShell.override { inherit stdenv; }) ({ inherit packages; } // env);
}
