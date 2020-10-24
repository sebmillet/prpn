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
	gettext.h MyIntl.h prpn.rc prpn.ico

bin_PROGRAMS=prpn prpnc
prpn_SOURCES=${MAINSOURCES}
prpnc_SOURCES=${MAINSOURCES}

AM_CPPFLAGS = @CFLAGS@ -Wshadow -Wall -Wextra -I$(srcdir)

Wx/SkHp28s.cpp: Wx/hp28sbg.cpp
Wx/SkHp28s2.cpp: Wx/hp28sbg2.cpp
Wx/SkHp28s3.cpp: Wx/hp28sbg3.cpp

# xd is not available by default. It converts a binary file into
# a file that can be compiled as C code (as an array of unsigned chars.)
# It can be found here (as of 17/04/2012):
#   http://www.fourmilab.ch/xd/
Wx/hp28sbg.cpp: Wx/hp28sbg.png
	xd -dhp28sbg Wx/hp28sbg.png > $@
	echo "unsigned long int nb_items_hp28sbg = sizeof(hp28sbg) / sizeof(*hp28sbg);" >> $@
Wx/hp28sbg2.cpp: Wx/hp28sbg2.png
	xd -dhp28sbg2 Wx/hp28sbg2.png > $@
	echo "unsigned long int nb_items_hp28sbg2 = sizeof(hp28sbg2) / sizeof(*hp28sbg2);" >> $@
Wx/hp28sbg3.cpp: Wx/hp28sbg3.png
	xd -dhp28sbg3 Wx/hp28sbg3.png > $@
	echo "unsigned long int nb_items_hp28sbg3 = sizeof(hp28sbg3) / sizeof(*hp28sbg3);" >> $@

AM_CPPFLAGS+= -DPKG_DOCDIR="\"${docdir}\"" -DPKG_HTMLDIR="\"${htmldir}\""
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

prpn_LDADD=
prpnc_LDADD=

if BUILD_CLIENTGUI

prpn_SOURCES+=Wx/UiImplWx.cpp Wx/SkBase.cpp Wx/SkBase.h \
	Wx/SkHp28s.cpp Wx/hp28sbg.png Wx/hp28sbg.cpp \
	Wx/SkHp28s2.cpp Wx/hp28sbg2.png Wx/hp28sbg2.cpp \
	Wx/SkHp28s3.cpp Wx/hp28sbg3.png Wx/hp28sbg3.cpp \
	Wx/xpm
prpn_CXXFLAGS=@WX_CXXFLAGS@ -DFOREIGN_MAIN -DPRPN_WX_GUI
prpn_LDADD+=@WX_LIBS@

endif

if NLS_EXPLICIT_LIB

prpn_LDADD+=-lintl
prpnc_LDADD+=-lintl

endif

if HAS_WINRES

prpn_LDADD+=prpnres.o
prpnc_LDADD+=prpnres.o
prpnres.o: prpn.rc
	rc=`$(WX_CONFIG_NAME) --rescomp`; \
	$${rc} $< -O coff -o $@

endif

