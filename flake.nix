{
  description = "CQ-SimBE reproducible C/C++ development environment";
  inputs.nixpkgs.url = "github:NixOS/nixpkgs/c7def046b9a883d46974757852106483d741586f";
  outputs = { self, nixpkgs }:
    let
      systems = [ "aarch64-darwin" "aarch64-linux" "x86_64-linux" ];
      eachSystem = nixpkgs.lib.genAttrs systems;
      mkEnvironment = { system, compiler ? "default", questOptions ? {}, packageOverrides ? (_: {}) }:
        import ./nix/environment.nix {
          pkgs = import nixpkgs { inherit system; };
          inherit compiler questOptions packageOverrides;
        };
    in {
      lib.mkEnvironment = mkEnvironment;
      devShells = eachSystem (system: {
        default = (mkEnvironment { inherit system; }).shell;
        clang = (mkEnvironment { inherit system; compiler = "clang"; }).shell;
        static-quest = (mkEnvironment { inherit system; questOptions.shared = false; }).shell;
        non-mpi = (mkEnvironment { inherit system; questOptions.enableMpi = false; }).shell;
      } // nixpkgs.lib.optionalAttrs (nixpkgs.lib.hasSuffix "linux" system) {
        gcc = (mkEnvironment { inherit system; compiler = "gcc"; }).shell;
      });
      packages = eachSystem (system:
        let environment = mkEnvironment { inherit system; }; in {
          inherit (environment.dependencies) quest unity mpi;
          default = environment.dependencies.quest;
        });
      checks = eachSystem (system:
        let pkgs = import nixpkgs { inherit system; }; in {
          environment-policy = pkgs.writeText "cq-simbe-environment-policy.json"
            (builtins.unsafeDiscardStringContext (builtins.toJSON
              (import ./nix/check-environment.nix { inherit nixpkgs; })));
        });
    };
}
