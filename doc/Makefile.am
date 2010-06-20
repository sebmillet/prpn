MANPAGE=${srcdir}/man/pRPN.1
HTMLPAGE=${srcdir}/pRPN.html
TEXTPAGE=${srcdir}/pRPN.txt
CMDSEN=${srcdir}/cmdsen.txt

dist_doc_DATA=${HTMLPAGE} ${TEXTPAGE}

EXTRA_DIST=${srcdir}/make-man-page.sh ${srcdir}/pRPN1-prefix ${srcdir}/pRPN1-postfix \
	${CMDSEN}

dist_man_MANS=${MANPAGE}


if BUILD_DOCS

# Building the page manual requires the program being built. BUT if we put a
# dependency with the executable, it'll trigger recreation of the manual page
# at every builds, that's not useful (the page manual is to be built in
# maintainer mode only.)
# The source is organized in a way that makes the manual page depend only on
# Commands.h. It results in a unusual but correct dependency chain ->
# according the Makefile, the manual page does not depend on the executable
# (though that's the reality) but on a piece of the executable source.
# You may wonder why we do not use a source (not C++) file for the command list
# or why we do not parse Commands.h.
# Parsing Commands.h would be OK but it'd be difficult to be independant of
# the code presentation, it would require to recreate a logic comparable to
# C++ preprocessor.
${MANPAGE}: ${CMDSEN} ${srcdir}/pRPN1-prefix ${srcdir}/pRPN1-postfix
	${srcdir}/make-man-page.sh "${srcdir}"

${HTMLPAGE}: ${MANPAGE}
	man2html ${MANPAGE} > $@

${TEXTPAGE}: ${MANPAGE}
	nroff -man ${MANPAGE} | col -b > $@

endif
