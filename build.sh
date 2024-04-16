#!/bin/sh

set -e
set -x

[ -d build ] || mkdir build

cd build

# TODO? make it work with system mp4v2
#cmake ../ -DWITH_MP4V2=/nix/store/x3i6x4a3j1va0ghb7c5ivwaw6ngzhymg-mp4v2-5.0.1
cmake ../

make
