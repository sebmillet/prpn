SUBDIRS = intl po src doc
ACLOCAL_AMFLAGS = -I admin

dist_doc_DATA=README

EXTRA_DIST=ABOUT-NLS admin/config.rpath test_linux extracfg.h wininst images \
	prpn.bkl src/dev.bkl \
	makefile.bcc makefile.gcc makefile.vc \
	src/makefile.bcc src/makefile.gcc src/makefile.vc \
	prpn.desktop prpn.png \
  bootstrap \
	prpn.spec gen-win-makefiles.cmd

dist-hook:
	rm -rf `find $(distdir) -name .svn`

appsdir = $(datadir)/applications
apps_DATA = prpn.desktop

icondir = $(datadir)/pixmaps
icon_DATA = prpn.png
