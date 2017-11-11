#!/bin/bash

rm -rf Makefile
rm -rf Makefile.in
rm -rf aclocal.m4
rm -rf autom4te.cache/
rm -rf aux-dist/
rm -rf compile
rm -rf config.h.in
rm -rf config.h
rm -rf config.log
rm -rf config.status
rm -rf configure
rm -rf depcomp
rm -rf include/Makefile
rm -rf include/Makefile.in
rm -rf install-sh
rm -rf libtool
rm -rf m4/
rm -rf missing
rm -rf src/*.la
rm -rf src/*.lo
rm -rf src/*.o
rm -rf src/.deps
rm -rf src/.libs
rm -rf src/Makefile
rm -rf src/Makefile.in
rm -rf src/libtool
rm -rf src/toollib/*.la
rm -rf src/toollib/*.lo
rm -rf src/toollib/.deps
rm -rf src/toollib/.dirstamp
rm -rf stamp-h1

rm -rf src/version.h
rm -rf src/tym

echo 'Cleaned files.'
