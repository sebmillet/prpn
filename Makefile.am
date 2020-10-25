SUBDIRS = po src doc
ACLOCAL_AMFLAGS = -I admin

dist_doc_DATA=README

EXTRA_DIST=ABOUT-NLS admin/config.rpath test_linux extracfg.h wininst images \
	prpn.desktop prpn.png \
	bootstrap

dist-hook:
	rm -rf `find $(distdir) -name .svn`
	rm -rf `find $(distdir) -name .git`

appsdir = $(datadir)/applications
apps_DATA = prpn.desktop

icondir = $(datadir)/pixmaps
icon_DATA = prpn.png
