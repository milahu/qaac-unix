{ pkgs ? import <nixpkgs> { } }:

with pkgs;

stdenv.mkDerivation {
  name = "qaac";
  src = ./.;
  buildInputs = [
    libcxx
  ];
}
