#!/bin/bash

set -eux

p=$(dirname "$0")

bash $p/cleanup.sh
autoreconf -fvi

# app image
sudo rm -rf AppDir
./configure --prefix="$PWD/AppDir/usr"
make
make install
sudo appimage-builder --generate
appimage-builder
