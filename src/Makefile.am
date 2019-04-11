bin_PROGRAMS = tym

tym_SOURCES = \
	common.h \
	tym.h \
	regex.h \
	common.c \
	option.c \
	config.c \
	keymap.c \
	hook.c \
	layout.c \
	builtin.c \
	command.c \
	context.c \
	app.c \
	tym.c

tym_LDADD = $(TYM_LIBS)

if DEBUG
ENV_OPT=-g3 -O0
else
ENV_OPT=-O3
endif

tym_CFLAGS = \
	-std=c11 \
	-Wall \
	-Wextra \
	-Wno-unused-parameter \
	-Wno-sign-compare \
	-Wno-pointer-sign \
	-Wno-missing-field-initializers \
	-Wformat=2 \
	-Wstrict-aliasing=2 \
	-Wdisabled-optimization \
	-Wfloat-equal \
	-Wpointer-arith \
	-Wbad-function-cast \
	-Wcast-align \
	-Wredundant-decls \
	-Winline \
	$(ENV_OPT) $(TYM_CFLAGS)
