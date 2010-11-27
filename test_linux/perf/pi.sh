#!/bin/sh

PRG=../../src/prpn
MYINPUT=tmp-input.txt

if [ -z "$2" ]; then
	echo "Usage: $0 undo_levels nb_decimals"
	exit 1
fi

echo $1 > $MYINPUT
echo "_UNDO_LEVELS" >> $MYINPUT
echo $2 >> $MYINPUT
cat pi.txt >> $MYINPUT

time $PRG -abz < $MYINPUT

rm $MYINPUT
