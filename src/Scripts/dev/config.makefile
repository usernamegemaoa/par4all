#
# $RCSfile: config.makefile,v $ for dev
#

SCRIPTS = 	pips-makemake \
		install_pips_sources \
		make-tags \
		pips_install_file \
		analyze_libraries \
		clean-pips \
		grep_libraries \
		search-pips \
		checked-out \
		order_libraries \
		pips-experiment \
		pips_at_night \
		remove_from_sccs_file

MACROS	=	define_libraries \
		makefile_macros.. \
		makefile_macros.DEFAULT \
		makefile_macros.GNU \
		makefile_macros.SUN4 \
		makefile_macros.TEST \
		makefile_macros.GNULL \
		makefile_macros.GNUSOL2LL

COPY	=	cp -f

INSTALL_UTL=	$(SCRIPTS)
INSTALL_INC=	$(MACROS)

SOURCES	=	$(SCRIPTS) $(MACROS) forward_gnu_makefile config.makefile

quick-install: install_forward_makefiles install_macros 
all: .runable

# bootstraping temporarily include files if needed...
$(PIPS_ROOT)/Include/makefile_macros.$(ARCH):
	touch $@

$(PIPS_ROOT)/Include/define_libraries:
	touch $@	

install_macros:
	#
	# installing makefile macros for pips/newgen/linear
	#
	# $(COPY) $(MACROS) $(PIPS_INCLUDEDIR)
	$(COPY) $(MACROS) $(NEWGEN_INCLUDEDIR)
	$(COPY) $(MACROS) $(LINEAR_INCLUDEDIR)

install_forward_makefiles: 
	# 
	# copying directory makefiles where required (and usefull)
	#
	# PIPS
	$(COPY) forward_gnu_makefile ${PIPS_ROOT}/Makefile
	$(COPY) forward_gnu_makefile ${PIPS_ROOT}/Src/Makefile
	$(COPY) forward_gnu_makefile ${PIPS_ROOT}/Src/Lib/Makefile
	$(COPY) forward_gnu_makefile ${PIPS_ROOT}/Src/Passes/Makefile
	$(COPY) forward_gnu_makefile ${PIPS_ROOT}/Src/Scripts/Makefile
	$(COPY) forward_gnu_makefile ${PIPS_ROOT}/Src/Runtime/Makefile
	#
	# NEWGEN
	$(COPY) forward_gnu_makefile ${NEWGEN_ROOT}/Makefile
	$(COPY) forward_gnu_makefile ${NEWGEN_ROOT}/Src/Makefile
	#
	# LINEAR
	$(COPY) forward_gnu_makefile ${LINEAR_ROOT}/Makefile
	$(COPY) forward_gnu_makefile ${LINEAR_ROOT}/Src/Makefile
	#
	# and development sides
	$(COPY) forward_gnu_makefile ${PIPS_DEVEDIR}/Makefile
	$(COPY) forward_gnu_makefile ${PIPS_DEVEDIR}/Lib/Makefile
	$(COPY) forward_gnu_makefile ${PIPS_DEVEDIR}/Passes/Makefile
	$(COPY) forward_gnu_makefile ${PIPS_DEVEDIR}/Scripts/Makefile
	$(COPY) forward_gnu_makefile ${PIPS_DEVEDIR}/Runtime/Makefile
	$(COPY) forward_gnu_makefile ${NEWGEN_DEVEDIR}/Makefile
	$(COPY) forward_gnu_makefile ${LINEAR_DEVEDIR}/Makefile

# that is all
#
