#!/bin/sh

PRG=../src/prpn

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

