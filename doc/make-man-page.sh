#!/bin/sh

# Sébastien Millet, 2010
# Generates help documents from binary executable (yes, really!)

SRCDIR="$1"
BASE=pRPN
MANDIR=man
MANPAGE=${MANDIR}/${BASE}.1
HTMLDOC=${BASE}.html
TEXTDOC=${BASE}.txt
CMDSEN=${SRCDIR}/cmdsen.txt
PREFIX=${SRCDIR}/pRPN1-prefix
POSTFIX=${SRCDIR}/pRPN1-postfix

mkdir -p $MANDIR

cat ${PREFIX} > ${MANPAGE}
egrep "\([[:digit:]]\):" ${CMDSEN} | sed 's/^\(\S\+\)\s\+(\(.\)):\s\+/.TP\n.B \1\\ (\2)\n/' >> ${MANPAGE}
echo ".SH \"OBJECT TYPES\"" >> ${MANPAGE}
echo ".TP" >> ${MANPAGE}
sed -n '/^Object type/,$p' ${CMDSEN} | egrep -v "get_class_count" | sed 's/$/\n.br/' | sed 's/^Object type/       Object type/' >> ${MANPAGE}
cat ${POSTFIX} >> ${MANPAGE}
