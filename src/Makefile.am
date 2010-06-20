EXENAME=prpn${EXEEXT}

bin_PROGRAMS=prpn
prpn_SOURCES=Main.cpp Compat.cpp Common.h \
	PGetOpt.cpp PGetOpt.h \
	Scalar.cpp Scalar.h \
	Stack.cpp Stack.h Commands.h \
	Variable.cpp Variable.h \
	Parser.cpp Parser.h \
	Ui.cpp  Ui.h \
	UiImplConsole.cpp \
	platform/os_generic.cpp platform/os_generic.h \
	platform/os_unknown.h platform/os_unix.h platform/os_win.h \
	gettext.h MyIntl.h

AM_CPPFLAGS=-DPKG_DOCDIR="\"${docdir}\"" -DPKG_HTMLDIR="\"${htmldir}\""
AM_CPPFLAGS+= -DPKG_PREFIX="\"${prefix}\"" -DPKG_DATADIR="\"${datadir}\""
AM_CPPFLAGS+= -DPKG_BINDIR="\"${bindir}\"" -DPKG_LOCALEDIR=\"$(localedir)\"

CMDS=${top_srcdir}/doc/cmds.txt
noinst_DATA=${CMDS}

if BUILD_DOCS

${CMDS}: Commands.h
	echo "_HELP" | ${builddir}/${EXENAME} -b > ${CMDS}

endif

if BUILD_CLIENTGUI

prpn_SOURCES+=UiImplWx.cpp \
	prpn.xpm \
	prpn.rc prpn.ico \
	shiftsel.xpm shiftuns.xpm \
	unit_bin.xpm unit_oct.xpm unit_dec.xpm unit_hex.xpm
prpn_CXXFLAGS=@WX_CXXFLAGS@ -DFOREIGN_MAIN
prpn_LDFLAGS=@WX_LIBS@

endif
