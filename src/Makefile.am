EXTRA_DIST = \
	Makefile.mingw

plugindir = $(libdir)/gaim

libnetsoul_la_LDFLAGS = -module -avoid-version

if PLUGINS

plugin_LTLIBRARIES = libnetsoul.la

libnetsoul_la_SOURCES = netsoul.c \
			netsoul.h \
			ns_buddy.c \
			ns_connection.c \
			ns_utils.c \
			ns_listen.c \
			ns_output.c \
			ns_chat.c

endif

AM_CPPFLAGS = \
	-DLIBDIR=\"$(libdir)/gaim/\" \
	-DDATADIR=\"$(datadir)\" \
	$(GLIB_CFLAGS) \
	$(GAIM_CFLAGS) \
	$(DEBUG_CFLAGS)

AM_CFLAGS=-DGAIM_PLUGINS
