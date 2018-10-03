bin_PROGRAMS = tym

tym_SOURCES = \
							common.h \
							tym.h \
							common.c \
							tym.c \
							app.c \
							command.c \
							config.c \
							context.c \
							builtin.c \
							keymap.c \
							option.c
tym_LDADD = $(TYM_LIBS)
if DEBUG
tym_CFLAGS = -std=c11 -Wall -Wextra -g3 -O0 $(TYM_CFLAGS)
else
tym_CFLAGS = -std=c11 -Wall -Wextra -O3 $(TYM_CFLAGS)
endif
