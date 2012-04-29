EXENAME=prpn${EXEEXT}

MAINSOURCES=Main.cpp Compat.cpp Common.h \
	PGetOpt.cpp PGetOpt.h \
	Scalar.cpp Scalar.h \
	Stack.cpp Stack.h Commands.h \
	Variable.cpp Variable.h \
	Parser.cpp Parser.h \
	Ui.cpp  Ui.h \
	UiImplConsole.cpp \
	MyString.cpp MyString.h \
	platform/os_generic.cpp platform/os_generic.h \
	platform/os_unknown.h platform/os_unix.h platform/os_win.h \
	gettext.h MyIntl.h

bin_PROGRAMS=prpn prpnc
prpn_SOURCES=${MAINSOURCES}
prpnc_SOURCES=${MAINSOURCES}

Wx/SkHp28s.cpp: Wx/SkHp28sb.cpp
Wx/SkHp28s2.cpp: Wx/SkHp28sb.cpp

# xd is not available by default. It converts a binary file into
# a file that can be compiled as C code (as an array of unsigned chars.)
# It can be found here (as of 17/04/2012):
#   http://www.fourmilab.ch/xd/
Wx/SkHp28sb.cpp: Wx/SkHp28sb.png
	xd -dhp28sb Wx/SkHp28sb.png > $@
	echo "size_t nb_items_hp28sb = sizeof(hp28sb) / sizeof(*hp28sb);" >> $@

AM_CPPFLAGS=-DPKG_DOCDIR="\"${docdir}\"" -DPKG_HTMLDIR="\"${htmldir}\""
AM_CPPFLAGS+= -DPKG_PREFIX="\"${prefix}\"" -DPKG_DATADIR="\"${datadir}\""
AM_CPPFLAGS+= -DPKG_BINDIR="\"${bindir}\"" -DPKG_LOCALEDIR=\"$(localedir)\"

CMDSEN=${top_srcdir}/doc/cmdsen.txt
CMDSFR=${top_srcdir}/doc/cmdsfr.txt
CMDSFRTMP=${top_srcdir}/doc/cmdsfr-0-tmp.txt
noinst_DATA=${CMDSEN} ${CMDSFR}

if BUILD_DOCS

${CMDSEN}: Commands.h
	echo "_HELP" | LC_ALL="C" ${builddir}/${EXENAME} -b > ${CMDSEN}

${CMDSFR}: Commands.h
	echo "_HELP" | LC_ALL="fr_FR.utf8" ${builddir}/${EXENAME} -b > ${CMDSFRTMP}
	iconv -f utf8 -t latin1 ${CMDSFRTMP} > ${CMDSFR}
	rm -f ${CMDSFRTMP}

endif

if BUILD_CLIENTGUI

prpn_SOURCES+=Wx/UiImplWx.cpp Wx/SkBase.cpp Wx/SkBase.h \
  Wx/SkHp28s.cpp Wx/SkHp28sb.png Wx/SkHp28sb.cpp Wx/SkHp28s2.cpp \
	Wx/xpm prpn.rc prpn.ico
prpn_CXXFLAGS=@WX_CXXFLAGS@ -DFOREIGN_MAIN
prpn_LDFLAGS=@WX_LIBS@

endif
