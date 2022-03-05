if DEBUG
ENV_OPT=-g3 -O0
else
ENV_OPT=-O3
endif

COMMON_CFLAGS = \
	$(ENV_OPT) \
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
	-Wformat-security \
	-Winline \
	-I$(top_srcdir)/include

bin_PROGRAMS = tym
tym_SOURCES = \
	app.c \
	builtin.c \
	command.c \
	common.c \
	config.c \
	context.c \
	hook.c \
	ipc.c \
	keymap.c \
	meta.c \
	option.c \
	property.c \
	tym.c
tym_LDADD = $(TYM_LIBS) $(LUA_LIBS)
tym_CFLAGS = $(COMMON_CFLAGS) $(TYM_CFLAGS) $(LUA_CFLAGS)

TESTS = tym-test
check_PROGRAMS = tym-test
tym_test_SOURCES = \
	app.c \
	builtin.c \
	command.c \
	common.c \
	config.c \
	context.c \
	hook.c \
	ipc.c \
	keymap.c \
	meta.c \
	option.c \
	property.c \
	config_test.c \
	option_test.c \
	regex_test.c \
	tym_test.c
tym_test_LDADD = $(TYM_LIBS) $(LUA_LIBS)
tym_test_CFLAGS = $(COMMON_CFLAGS) $(TYM_CFLAGS) $(LUA_CFLAGS)
