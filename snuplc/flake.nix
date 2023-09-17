{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let pkgs = nixpkgs.legacyPackages.${system}; in
      {
        packages.default = pkgs.stdenv.mkDerivation {
          name = "snuplc";
          src = ./.;
          nativeBuildInputs = with pkgs; [ doxygen graphviz ];

          outputs = [ "out" "doc" ];
          buildFlags = [ "all" "doc" ];

          installPhase = ''
            runHook preInstall
            install -Dm755 test_scanner $out/bin/test_scanner
            mkdir -p $doc
            cp -r doc/html $doc/html
            runHook postInstall
          '';

          meta.mainProgram = "test_scanner";
        };
      }
    );
}
