#!/bin/bash

set -eux

bash cleanup.sh
autoreconf -fvi
./configure
make clean
make
make dist
