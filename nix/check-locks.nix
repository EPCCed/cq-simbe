# Both entry points reject dependency drift before entering the shell.
let
  flake = (builtins.fromJSON (builtins.readFile ../flake.lock)).nodes.nixpkgs.locked;
  devenv = (builtins.fromJSON (builtins.readFile ../devenv.lock)).nodes.nixpkgs.locked;
in
assert flake.rev == "c7def046b9a883d46974757852106483d741586f";
assert flake.rev == devenv.rev;
assert flake.narHash == devenv.narHash;
true
