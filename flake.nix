{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let pkgs = nixpkgs.legacyPackages.${system}; in
      {
        devShells.default = pkgs.mkShell {
          inputsFrom = [ self.packages.${system}.default ];
          nativeBuildInputs = with pkgs; [ man-pages valgrind ];
          hardeningDisable = [ "all" ];
        };

        packages.default = pkgs.stdenv.mkDerivation {
          name = "snuplc";
          src = ./snuplc;
          nativeBuildInputs = with pkgs; [ doxygen graphviz ];

          outputs = [ "out" "doc" ];
          buildFlags = [ "all" "doc" ];

          enableParallelBuilding = true;

          installPhase = ''
            runHook preInstall
            install -Dm755 -t $out/bin/ test_*
            mkdir -p $doc
            cp -r doc/html $doc/html
            runHook postInstall
          '';

          hardeningDisable = [ "all" ];

          meta.mainProgram = "test_scanner";
        };

        packages.ref = pkgs.gcc13Stdenv.mkDerivation {
          name = "snuplc-ref";
          src = ./snuplc/reference;

          nativeBuildInputs = with pkgs; [ autoPatchelfHook ];
          buildInputs = with pkgs; [ gcc13Stdenv.cc.cc.lib ];

          installPhase = ''
            runHook preInstall
            install -Dm755 -t $out/bin/ test_*
            runHook postInstall
          '';
        };
      }
    );
}
