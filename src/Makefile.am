include $(top_srcdir)/common.am
bin_PROGRAMS = tym
tym_SOURCES = \
	app.c \
	builtin.c \
	command.c \
	common.c \
	config.c \
	context.c \
	hook.c \
	keymap.c \
	layout.c \
	meta.c \
	option.c \
	property.c \
	tym.c
tym_LDADD = $(TYM_LIBS)
tym_CFLAGS = $(COMMON_CFLAGS) $(TYM_CFLAGS)
