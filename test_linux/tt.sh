#!/bin/sh

PRG=../src/prpn

VERSION=`$PRG --version 2>&1 | grep -i "^prpn [0-9.]\+"`
VERSION=`echo $VERSION | sed 's/^prpn *//i;s/ .*$//'`
ISDEBUG=`echo $VERSION | sed 's/.*\(.\)$/\1/'`
ISDEBUG=`echo $ISDEBUG | sed 's/^\([^d]\)$/0/'`
ISDEBUG=`echo $ISDEBUG | sed 's/^\([d]\)$/1/'`

echo "Executable: $PRG"
echo "Executable version: $VERSION"

if [ $ISDEBUG -eq 1 ]; then
	echo "** FATAL"
	echo "Program has been compiled with debug option (./configure --enable-debug)"
	echo "Tests need a non-debug compiled executable. Aborting."
	exit 1
fi

if [ -x "$PRG" ]; then
	for f in `ls -d t[0-9][0-9]`; do
		cd $f
		./test.sh --batch
		cd ..
	done
else
	echo "$PRG not available"
	exit 1
fi

