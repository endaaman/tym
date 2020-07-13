#!/bin/bash

set -eux

rm -f Makefile
rm -f Makefile.in
rm -f aclocal.m4
rm -f compile
rm -f config.h
rm -f config.h.in
rm -f config.h.in~
rm -f config.log
rm -f config.status
rm -f configure
rm -f depcomp
rm -f install-sh
rm -f libtool
rm -f missing
rm -f stamp-h1
rm -rf autom4te.cache/
rm -rf aux-dist/
rm -rf m4/
rm -f src/*.la
rm -f src/*.lo
rm -f src/*.o
rm -f src/Makefile
rm -f src/Makefile.in
rm -f src/tym
rm -rf src/.deps/

rm -f src/common.h

rm -f src/*.log
rm -f src/*.trs
rm -f src/tym-test

rm -f tym.1
rm -f test-driver

echo 'Cleaned files.'
