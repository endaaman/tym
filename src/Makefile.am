bin_PROGRAMS = tym


tym_SOURCES = tym.c
tym_LDADD = $(TYM_LIBS)
if DEBUG
tym_CFLAGS = -std=c11 -Wall -Wextra -g3 -O0 $(TYM_CFLAGS)
else
tym_CFLAGS = -std=c11 -Wall -Wextra -O3 $(TYM_CFLAGS)
endif
