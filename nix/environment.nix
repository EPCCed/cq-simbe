# One compiler and dependency definition for devenv shell and nix develop.
{ pkgs, compiler ? "default", questOptions ? {}, packageOverrides ? (_: {}) }:
assert import ./check-locks.nix;
let
  inherit (pkgs) lib;
  darwin = pkgs.stdenv.hostPlatform.isDarwin;
  stdenv = if darwin then pkgs.llvmPackages_22.libcxxStdenv
    else if compiler == "clang" then pkgs.llvmPackages_22.stdenv
    else pkgs.stdenv;
  base = final: {
    openmp = pkgs.llvmPackages_22.openmp;
    mpi = (pkgs.mpich.override { inherit stdenv; }).overrideAttrs (old:
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
  cmakeDependencies = with dependencies; [ quest unity ]
    ++ lib.optional dependencies.quest.enableMpi dependencies.mpi
    ++ lib.optionals (dependencies.quest.enableOpenmp && stdenv.cc.isClang) [ dependencies.openmp ]
    ++ lib.optional (dependencies.quest.enableOpenmp && dependencies.quest.enableNuma) pkgs.numactl;
  packages = cmakeDependencies ++ [ pkgs.cmake pkgs.ninja pkgs.git pkgs.pkg-config ];
  env = {
    CC = "${stdenv.cc}/bin/${stdenv.cc.targetPrefix}cc";
    CXX = "${stdenv.cc}/bin/${stdenv.cc.targetPrefix}c++";
    CMAKE_PREFIX_PATH = lib.makeSearchPath "" (map lib.getDev cmakeDependencies);
  };
in
assert lib.assertMsg (builtins.elem compiler [ "default" "gcc" "clang" ]) "Unknown compiler selection.";
assert lib.assertMsg (!darwin || compiler != "gcc") "The GCC shell is supported on Linux; Darwin uses LLVM 22/libc++.";
assert pkgs.cmake.version == "4.4.2";
{
  inherit stdenv dependencies cmakeDependencies packages env;
  shell = (pkgs.mkShell.override { inherit stdenv; }) ({ inherit packages; } // env);
}
