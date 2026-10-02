# Evaluation checks cover dependency selection; native builds remain CI gates.
{ nixpkgs }:
let
  lib = nixpkgs.lib;
  check = system:
    let
      pkgs = import nixpkgs { inherit system; };
      environment = import ./environment.nix { inherit pkgs; };
      static = import ./environment.nix { inherit pkgs; questOptions.shared = false; };
      serial = import ./environment.nix { inherit pkgs; questOptions.enableMpi = false; };
      clang = import ./environment.nix { inherit pkgs; compiler = "clang"; };
      deps = environment.dependencies;
    in
    assert deps.quest.options.QUEST_FLOAT_PRECISION == 2;
    assert deps.quest.shared && deps.quest.enableMpi && deps.quest.enableSubcomm && deps.quest.enableOpenmp;
    assert deps.quest.enableNuma == pkgs.stdenv.hostPlatform.isLinux;
    assert deps.quest.options.CMAKE_POSITION_INDEPENDENT_CODE;
    assert deps.quest.options.QUEST_ENABLE_INSTALL && !deps.quest.options.QUEST_ENABLE_PACKAGING;
    assert !static.dependencies.quest.shared && static.dependencies.quest.options.CMAKE_POSITION_INDEPENDENT_CODE;
    assert !serial.dependencies.quest.enableMpi && !serial.dependencies.quest.enableSubcomm;
    assert !(builtins.elem serial.dependencies.mpi serial.cmakeDependencies);
    assert deps.unity.version == "2.6.1";
    assert deps.quest.stdenv.drvPath == environment.stdenv.drvPath;
    assert deps.unity.stdenv.drvPath == environment.stdenv.drvPath;
    assert deps.mpi.stdenv.drvPath == environment.stdenv.drvPath;
    assert clang.dependencies.quest.stdenv.drvPath == clang.stdenv.drvPath;
    assert clang.dependencies.mpi.stdenv.drvPath == clang.stdenv.drvPath;
    assert clang.stdenv.cc.isClang;
    assert if pkgs.stdenv.hostPlatform.isDarwin then
      environment.stdenv.drvPath == pkgs.llvmPackages_22.libcxxStdenv.drvPath
    else environment.stdenv.drvPath == pkgs.stdenv.drvPath;
    {
      inherit system;
      compiler = environment.env.CXX;
      prefix = environment.env.CMAKE_PREFIX_PATH;
      quest = deps.quest.drvPath;
      unity = deps.unity.drvPath;
      mpi = deps.mpi.drvPath;
      clangShell = clang.shell.drvPath;
    };
in builtins.map check [ "aarch64-darwin" "aarch64-linux" "x86_64-linux" ]
