TARGET	= pipsmake
include_HEADERS=$(TARGET).h
BUILT_SOURCES=$(TARGET).h readmakefile.h
include $(top_srcdir)/makes/cproto.mk
noinst_LTLIBRARIES=libpipsmake.la
libpipsmake_la_SOURCES=\
		readmakefile.y\
		lexer.l\
		pipsmake.c \
		activate.c \
		openclose.c \
		callback.c \
		unsplit.c \
		stubs.c

AM_YFLAGS=-d -p pipsmake_
AM_LFLAGS=-p pipsmake_

CLEANFILES=readmakefile.[ch] lexer.c

include $(srcdir)/../pipslibs_includes.mk
