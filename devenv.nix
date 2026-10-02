{ pkgs, lib, config, ... }:
let
  environment = import ./nix/environment.nix {
    inherit pkgs;
    inherit (config.cq-simbe) compiler enableMpi questOptions packageOverrides;
  };
in {
  options.cq-simbe = {
    compiler = lib.mkOption {
      type = lib.types.enum [ "default" "gcc" "clang" ];
      default = "default";
      description = "Compiler selection; override in devenv.local.nix.";
    };
    enableMpi = lib.mkOption {
      type = lib.types.bool;
      default = true;
      description = "Provide MPI for CQ transport, independently of QuEST's MPI setting.";
    };
    questOptions = lib.mkOption {
      type = lib.types.attrs;
      default = {};
      description = "Arguments to nix/quest.nix; override in devenv.local.nix.";
    };
    packageOverrides = lib.mkOption {
      type = lib.types.functionTo lib.types.attrs;
      default = _: {};
      description = "Override dependencies in the shared environment.";
    };
  };
  config = {
    stdenv = environment.stdenv;
    packages = environment.packages;
    env = environment.env;
  };
}
