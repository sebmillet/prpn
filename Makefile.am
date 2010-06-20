SUBDIRS = src doc
ACLOCAL_AMFLAGS = -I admin

dist_doc_DATA=README

EXTRA_DIST=test_linux extracfg.h wininst images

dist-hook:
	rm -rf `find $(distdir) -name .svn`
