# $Id$

include $(ROOT)/makes/GNULL.mk

# -ansi needed for ... declarations?
CC	= gcc-2.8.1 -ansi

# posix needed to define signal macros.
# xopen needed for timeval
CPPFLAGS+= 	-D_INCLUDE_POSIX_SOURCE \
		-D_INCLUDE_XOPEN_SOURCE \
		-D_INCLUDE_XOPEN_SOURCE_EXTENDED \
		-D_PROTOTYPES \
		-I$(HOME)/include

LDFLAGS	+=	-L$(HOME)/lib

AR	= ar
RANLIB	= ranlib
