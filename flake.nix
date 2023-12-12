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
          inputsFrom = with self.packages.${system}; [ default reports ];
          nativeBuildInputs = with pkgs; [ man-pages valgrind ];
          hardeningDisable = [ "all" ];
        };

        packages = {
          default = pkgs.stdenv.mkDerivation {
            name = "snuplc";
            src = ./snuplc;

            enableParallelBuilding = true;

            installPhase = ''
              runHook preInstall
              install -Dm755 snuplc $out/bin/snuplc
              install -Dm644 -t $out/bin/rte/x86-64/ rte/x86-64/*.o
              runHook postInstall
            '';

            hardeningDisable = [ "all" ];
          };

          reports = pkgs.stdenvNoCC.mkDerivation {
            name = "snuplc-reports";
            src = ./reports;

            nativeBuildInputs = with pkgs; [ pandoc texliveMedium ];

            installPhase = ''
              runHook preInstall
              install -Dm644 -t $out *.pdf
              runHook postInstall
            '';
          };

          ref = pkgs.gcc13Stdenv.mkDerivation {
            name = "snuplc-ref";
            src = ./snuplc/reference;

            nativeBuildInputs = with pkgs; [ autoPatchelfHook ];
            buildInputs = with pkgs; [ gcc13Stdenv.cc.cc.lib ];

            installPhase = ''
              runHook preInstall
              install -Dm755 -t $out/bin/ snuplc
              runHook postInstall
            '';
          };
        };
      }
    );
}
