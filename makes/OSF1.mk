# $Id$

include $(ROOT)/makes/DEFAULT.mk

RANLIB	= :

LEX	= flex
LFLAGS	=
FC	= f90
FFLAGS	= -O agress -O 3 -O unroll2 -O split2 -O scalar3 -e I
# There is a bug in DEC OSF/1 on syntax/syn_yacc.c :

# the cray m4 results in some internal stack overflow on hpfc runtime...
M4	= gm4
