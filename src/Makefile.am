## Process this file with automake to produce Makefile.in

INCLUDES = \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	@PACKAGE_CFLAGS@

bin_PROGRAMS = rugen

rugen_SOURCES = \
	main.c \
	support.c support.h \
	interface.c interface.h \
	callbacks.c callbacks.h \
	dv1394app.c dv1394app.h \
	page.c page.h \
	page_clips.c \
	page_command.c \
	page_status.c \
	page_units.c \
	gtkenhancedscale.c gtkenhancedscale.h

rugen_CFLAGS = @PACKAGE_CFLAGS@
rugen_LDADD = @PACKAGE_LIBS@

