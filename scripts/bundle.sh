#!/bin/bash

set -eux

p=$(dirname "$0")

bash $p/cleanup.sh
autoreconf -fvi
./configure
make clean
make
make dist
