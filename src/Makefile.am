if DEBUG
ENV_OPT=-g3 -O0
else
ENV_OPT=-O3
endif

CFLAGS_COMMON = \
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
	$(ENV_OPT)

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
tym_CFLAGS = $(CFLAGS_COMMON) $(TYM_CFLAGS)

TESTS = tym_test_regex
check_PROGRAMS = tym_test_regex
tym_test_regex_SOURCES = regex.h test_regex.c
tym_test_regex_LDADD = $(TYM_TEST_REGEX_LIBS)
tym_test_regex_CFLAGS = $(CFLAGS_COMMON) $(TYM_TEST_REGEX_CFLAGS)
