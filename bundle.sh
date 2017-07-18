#!/bin/bash

bash cleanup.sh
autoreconf -fvi
./configure
make clean
make
make dist
