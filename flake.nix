rec {
  description = "TTF to PNG Font Generator";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs?ref=nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
      in rec {
        packages = rec {
          default = fontgen;

          fontgen = pkgs.stdenv.mkDerivation {
            pname = "fontgen";
            version = "0.0.0";

            src = ./.;

            nativeBuildInputs = with pkgs; [
              cmake
            ];

            buildInputs = with pkgs; [
              freetype
            ];
           };
        };
      }
    );
}
