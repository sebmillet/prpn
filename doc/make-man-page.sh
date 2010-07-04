#!/bin/sh

# Sébastien Millet, 2010
# Generates help documents from binary executable (yes, really!)

SRCDIR="$1"
BASE=pRPN

for LG in en fr; do
	if [ ${LG} = "en" ]; then
		MANDIR=man
	else
		MANDIR=man/${LG}
	fi
	MANPAGE=${MANDIR}/${BASE}.1
	CMDS=${SRCDIR}/cmds${LG}.txt
	PREFIX=${SRCDIR}/pRPN1${LG}-prefix
	POSTFIX=${SRCDIR}/pRPN1${LG}-postfix
	HTMLDOC=${BASE}${LG}.html
	TEXTDOC=${BASE}${LG}.txt
	mkdir -p $MANDIR
	cat ${PREFIX} > ${MANPAGE}
	egrep "\([[:digit:]]\):" ${CMDS} | sed 's/^\(\S\+\)\s\+(\(.\)):\s\+/.TP\n.B \1\\ (\2)\n/' >> ${MANPAGE}

#	echo ".SH \"OBJECT TYPES\"" >> ${MANPAGE}

	echo ".TP" >> ${MANPAGE}

#	sed -n '/^Object type/,$p' ${CMDS} | egrep -v "get_class_count" | sed 's/$/\n.br/' | sed 's/^Object type/       Object type/' >> ${MANPAGE}

	cat ${POSTFIX} >> ${MANPAGE}
done
