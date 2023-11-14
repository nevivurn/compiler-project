{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let pkgs = nixpkgs.legacyPackages.${system}; in
      {
        packages.default = pkgs.stdenvNoCC.mkDerivation {
          name = "snucc-reports";
          src = ./.;
          nativeBuildInputs = with pkgs; [ pandoc texlive.combined.scheme-medium ];

          buildPhase = ''
            runHook preBuild
            for f in *.md; do
              pandoc "$f" -o "''${f%.md}.pdf"
            done
            runHook postBuild
          '';

          installPhase = ''
            runHook preInstall
            install -Dm644 -t $out *.pdf
            runHook postInstall
          '';
        };
      }
    );
}
