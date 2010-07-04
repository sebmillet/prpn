#!/bin/sh

# Sébastien Millet, 2010
# Generates help documents from binary executable (yes, really!)

SRCDIR="$1"
LG="$2"
BASE=pRPN

if [ ${LG} = "en" ]; then
	MANPAGE=${BASE}.1
else
	MANPAGE=${BASE}.${LG}.1
fi
CMDS=${SRCDIR}/cmds${LG}.txt
PREFIX=${SRCDIR}/pRPN1${LG}-prefix
POSTFIX=${SRCDIR}/pRPN1${LG}-postfix
HTMLDOC=${BASE}${LG}.html
TEXTDOC=${BASE}${LG}.txt
cat ${PREFIX} > ${MANPAGE}
egrep "\([[:digit:]]\):" ${CMDS} | sed 's/^\(\S\+\)\s\+(\(.\)):\s\+/.TP\n.B \1\\ (\2)\n/' >> ${MANPAGE}
cat ${POSTFIX} >> ${MANPAGE}
