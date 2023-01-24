#!/bin/bash

set -eux

rm -f Makefile
rm -f Makefile.in
rm -f aclocal.m4
rm -f app-config.h
rm -f app-config.h.in
rm -f compile
rm -f config.guess
rm -f config.h
rm -f config.h.in
rm -f config.h.in~
rm -f config.log
rm -f config.status
rm -f config.sub
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
rm -f src/*.log
rm -f src/*.o
rm -f src/*.trs
rm -f src/Makefile
rm -f src/Makefile.in
rm -f src/tym
rm -f src/tym-test
rm -rf src/.deps/

rm -f include/Makefile
rm -f include/Makefile.in
rm -f include/common.h

rm -f tym.1
rm -f tym-daemon.service
rm -f test-driver

echo 'Cleaned files.'
