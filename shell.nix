{ pkgs ? import <nixpkgs> {} }:

with pkgs;

mkShell {
  buildInputs = [
    #gnumake # make
    cmake
    pkg-config

    # clang gives better error mesages than gcc
    #gcc
    clang # llvmPackages_16.clang
    #llvmPackages_17.clang

    #python3 # for node-gyp

    #mp4v2
    #(callPackage ./nix/mp4v2 { })

    taglib
    libsndfile
    flac
    libogg # for flac
    audiofile
    soxr
    libuchardet

    # use dll files on linux
    #(callPackage ./nix/loadlibrary { })
  ];

  shellHook = ''
    export CC=clang
    export CXX=clang++
  '';
}
