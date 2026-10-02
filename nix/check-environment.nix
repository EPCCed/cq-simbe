# Evaluation checks cover dependency selection; native builds remain CI gates.
{ nixpkgs }:
let
  lib = nixpkgs.lib;
  implicitLinkExclusions = [
    "/lib" "/lib32" "/lib64" "/usr/lib" "/usr/lib32" "/usr/lib64"
    "/lib/x86_64-linux-gnu" "/usr/lib/x86_64-linux-gnu"
    "/lib/aarch64-linux-gnu" "/usr/lib/aarch64-linux-gnu"
  ];
  checkImplicitLinkExclusions = environment: enabled:
    lib.all (language:
      let name = "CMAKE_${language}_IMPLICIT_LINK_DIRECTORIES_EXCLUDE"; in
      if enabled then
        lib.splitString ";" (environment.env.${name} or "") == implicitLinkExclusions
      else !(builtins.hasAttr name environment.env)
    ) [ "C" "CXX" ];
  check = system:
    let
      pkgs = import nixpkgs { inherit system; };
      environment = import ./environment.nix { inherit pkgs; };
      static = import ./environment.nix { inherit pkgs; questOptions.shared = false; };
      serial = import ./environment.nix { inherit pkgs; questOptions.enableMpi = false; };
      nonMpi = import ./environment.nix { inherit pkgs; enableMpi = false; questOptions.enableMpi = false; };
      clang = import ./environment.nix { inherit pkgs; compiler = "clang"; };
      gcc = import ./environment.nix { inherit pkgs; compiler = "gcc"; };
      deps = environment.dependencies;
    in
    assert deps.quest.options.QUEST_FLOAT_PRECISION == 2;
    assert deps.quest.shared && deps.quest.enableMpi && deps.quest.enableSubcomm && deps.quest.enableOpenmp;
    assert deps.quest.enableNuma == pkgs.stdenv.hostPlatform.isLinux;
    assert deps.quest.options.CMAKE_POSITION_INDEPENDENT_CODE;
    assert deps.quest.options.QUEST_ENABLE_INSTALL && !deps.quest.options.QUEST_ENABLE_PACKAGING;
    assert !static.dependencies.quest.shared && static.dependencies.quest.options.CMAKE_POSITION_INDEPENDENT_CODE;
    assert !serial.dependencies.quest.enableMpi && !serial.dependencies.quest.enableSubcomm;
    assert builtins.elem serial.dependencies.mpi serial.cmakeDependencies;
    assert !(builtins.elem nonMpi.dependencies.mpi nonMpi.cmakeDependencies);
    assert !nonMpi.dependencies.quest.enableMpi;
    assert deps.fortran.drvPath == pkgs.gfortran.drvPath;
    assert environment.env.FC == "${deps.fortran}/bin/gfortran";
    assert builtins.elem deps.nlopt environment.cmakeDependencies;
    assert deps.nlopt.version == "2.11.0";
    assert deps.unity.version == "2.6.1";
    assert deps.quest.stdenv.drvPath == environment.stdenv.drvPath;
    assert deps.unity.stdenv.drvPath == environment.stdenv.drvPath;
    assert deps.mpi.stdenv.drvPath == environment.stdenv.drvPath;
    assert clang.dependencies.quest.stdenv.drvPath == clang.stdenv.drvPath;
    assert clang.dependencies.mpi.stdenv.drvPath == clang.stdenv.drvPath;
    assert clang.stdenv.cc.isClang;
    assert checkImplicitLinkExclusions clang pkgs.stdenv.hostPlatform.isLinux;
    assert builtins.all (environment: checkImplicitLinkExclusions environment false)
      [ environment static serial nonMpi ];
    assert pkgs.stdenv.hostPlatform.isDarwin || checkImplicitLinkExclusions gcc false;
    assert if pkgs.stdenv.hostPlatform.isDarwin then
      environment.stdenv.drvPath == pkgs.llvmPackages_22.libcxxStdenv.drvPath
    else environment.stdenv.drvPath == pkgs.stdenv.drvPath;
    {
      inherit system;
      compiler = environment.env.CXX;
      fortranCompiler = environment.env.FC;
      prefix = environment.env.CMAKE_PREFIX_PATH;
      quest = deps.quest.drvPath;
      unity = deps.unity.drvPath;
      mpi = deps.mpi.drvPath;
      nlopt = deps.nlopt.drvPath;
      clangShell = clang.shell.drvPath;
    };
in builtins.map check [ "aarch64-darwin" "aarch64-linux" "x86_64-linux" ]
