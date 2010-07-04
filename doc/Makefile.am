# EN
MANPAGEEN=${srcdir}/pRPN.1
HTMLPAGEEN=${srcdir}/pRPNen.html
TEXTPAGEEN=${srcdir}/pRPNen.txt
CMDSEN=${srcdir}/cmdsen.txt

# FR
MANPAGEFR=${srcdir}/pRPN.fr.1
HTMLPAGEFR=${srcdir}/pRPNfr.html
TEXTPAGEFR=${srcdir}/pRPNfr.txt
CMDSFR=${srcdir}/cmdsfr.txt

dist_doc_DATA=${HTMLPAGEEN} ${TEXTPAGEEN} \
	${HTMLPAGEFR} ${TEXTPAGEFR}

EXTRA_DIST=${srcdir}/make-man-page.sh ${srcdir}/pRPN1-prefix ${srcdir}/pRPN1-postfix \
	${CMDSEN} ${CMDSFR}

manfrdir = $(mandir)/fr
dist_man_MANS=${MANPAGEEN} ${MANPAGEFR}


if BUILD_DOCS

# Building the page manual requires the program being built. BUT if we put a
# dependency with the executable, it'll trigger recreation of the manual page
# at every builds, that's not useful (the page manual is to be built in
# maintainer mode only.)
# The source is organized in a way that makes the manual page depend only on
# Commands.h. It results in an unusual but correct dependency chain ->
# according to the Makefile, the manual page does not depend on the executable
# (though that's the reality) but on a piece of the executable source.
# You may wonder why we do not use a source (not C++) file for the command list
# or why we do not parse Commands.h.
# Parsing Commands.h would be OK but it'd be difficult to be independant of
# the code presentation, it would require to recreate a logic comparable to
# C++ preprocessor.

# EN
${MANPAGEEN}: ${CMDSEN} ${srcdir}/pRPN1en-prefix ${srcdir}/pRPN1en-postfix ${srcdir}/make-man-page.sh
	${srcdir}/make-man-page.sh "${srcdir}" en
${HTMLPAGEEN}: ${MANPAGEEN}
	man2html ${MANPAGEEN} > $@
${TEXTPAGEEN}: ${MANPAGEEN}
	nroff -man ${MANPAGEEN} | col -b > $@

# FR
${MANPAGEFR}: ${CMDSFR} ${srcdir}/pRPN1fr-prefix ${srcdir}/pRPN1fr-postfix ${srcdir}/make-man-page.sh
	${srcdir}/make-man-page.sh "${srcdir}" fr
${HTMLPAGEFR}: ${MANPAGEFR}
	man2html ${MANPAGEFR} > $@
${TEXTPAGEFR}: ${MANPAGEFR}
	nroff -man ${MANPAGEFR} | col -b > $@

endif
