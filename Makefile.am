SUBDIRS = intl po src doc
ACLOCAL_AMFLAGS = -I admin

dist_doc_DATA=README

EXTRA_DIST= admin/config.rpath test_linux extracfg.h wininst images \
	prpn.bkl src/dev.bkl \
	makefile.bcc makefile.gcc \
	src/makefile.bcc src/makefile.gcc

dist-hook:
	rm -rf `find $(distdir) -name .svn`
