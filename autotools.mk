all:

clean:
	rm -f configure Makefile.in config.h.in aclocal.m4
	rm -f install-sh missing depcomp compile
	rm -rf autom4te.cache
	rm -f *~

rescan:
	autoscan

reconfigure:
	autoreconf -i

configure:	configure.in aclocal.m4 Makefile.in config.h.in
	autoconf

Makefile.in:	Makefile.am config.h.in
	automake -a -c

config.h.in:	configure.in
	autoheader

aclocal.m4:	configure.in
	aclocal
