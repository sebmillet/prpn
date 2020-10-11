#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

# Sébastien Millet, April 2012, July 2018, October 2020

AC_PREREQ([2.65])

AC_INIT([pRPN], [0.6.0], [milletseb@laposte.net])

AC_CONFIG_SRCDIR([src/Common.h])
AC_CONFIG_AUX_DIR([admin])

AC_CANONICAL_BUILD
AC_CANONICAL_HOST

AC_USE_SYSTEM_EXTENSIONS

AM_INIT_AUTOMAKE([subdir-objects])
AM_GNU_GETTEXT([external])

AM_MAINTAINER_MODE

AC_CONFIG_HEADERS([config.h])

dnl ==================== Options ====================================

AC_ARG_ENABLE(debug,
              [  --enable-debug          enable debugging information],
              enable_debug="$enableval", enable_debug="no")

if test "x$USE_MAINTAINER_MODE" = "xyes"; then
	build_docs=yes
else
	build_docs=no
fi

if test "x$build_docs" = "xyes"; then
	if test "x$enable_debug" = "xyes"; then
		AC_MSG_ERROR([[unable to run in maintainer mode while DEBUG is enabled]])
	fi
fi

AM_CONDITIONAL(BUILD_DOCS, test $build_docs = yes)

AC_ARG_ENABLE(gui,
[  --disable-gui           do not build GUI version of program])
test -z "$enable_gui" && enable_gui="yes"
AM_CONDITIONAL([ENABLEGUI], [test x$enable_gui = xyes])

if test "${enable_debug}" = yes ; then
  CPPFLAGS="$CPPFLAGS -DDEBUG"
fi

dnl ==================WxWidgets Options =============================

if test "${enable_gui}" = yes ; then

AM_OPTIONS_WXCONFIG
AM_PATH_WXCONFIG(3.0.0, wxWin=1)

if ( test "$wxWin" != 1 ); then
   AC_MSG_ERROR([
================================================================================
ERROR: A suitable installation of wxWidgets could not be found 

  If you add wxWidgets to your system, then this configure script will also
  configure your system to build the BOINC graphical client (clientgui).  

  If wxWidgets is installed on your system, please check that wx-config is 
  in the path, that the directory where wxWidgets libraries are installed 
  (returned by 'wx-config --libs' command) is in LD_LIBRARY_PATH (or equivalent),
  and that wxWidgets version is 2.6.0 or above. Currently wxWidgets version
  2.6.0 (gtk based) is known to work with boinc_gui under Linux. You can
  use wx-config --version to find what version you have currently installed.

  You can get wxWidgets by following the DOWNLOAD link at:
  http://www.wxwindows.org/

  You can also compile this program without a GUI, execute:
    ./configure --disable-gui

================================================================================
       ])
fi

fi

AM_CONDITIONAL(BUILD_CLIENTGUI, [ test "$wxWin" = 1 ])

AC_SUBST([WX_CXXFLAGS])
AC_SUBST([WX_LIBS])

# Hack to successfully link wuth gcc version 4.6, cf. URL
# http://ubuntuforums.org/showthread.php?p=11347972&posted=1#post11347972
# It is gruiiik, but don't know how to change the options order
# in g++ call.
CXXFLAGS="$CXXFLAGS -Wl,--no-as-needed"

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_SIZE_T

# Checks for library functions.
AC_CHECK_FUNCS([sleep])

AC_OUTPUT(Makefile src/Makefile doc/Makefile po/Makefile.in)

