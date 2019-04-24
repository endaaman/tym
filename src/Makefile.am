include $(top_srcdir)/common.am
bin_PROGRAMS = tym
tym_SOURCES = \
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
tym_CFLAGS = $(COMMON_CFLAGS) $(TYM_CFLAGS)
