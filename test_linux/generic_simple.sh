#!/bin/sh

PRG=../../src/prpn

TNAME=$1
INPUT=$2
OUTPUT=$3
REFERENCE=$4

$PRG -abz < $INPUT > $OUTPUT 2>&1
sed -i -e "/Warning: Mismatch between the program and library build versions \
detected/d" $OUTPUT
sed -i -e "/The library used .* (wchar_t,compiler with C++ ABI .*,wx \
containers,compatible with .*)/d" $OUTPUT
sed -i -e "/and your program used .* (wchar_t,compiler with C++ ABI .*,wx \
containers,compatible with .*)/d" $OUTPUT

REP=$(pwd | sed 's/.*\///')

if [ "$5" = "--batch" ]; then
	cmp $REFERENCE $OUTPUT 2>&1 > /dev/null
	if [ "$?" -ne "0" ]; then
		echo "$REP ** $TNAME: KO"
		exit 1;
	else
		echo "$REP    $TNAME: OK"
		exit 0;
	fi
fi

cat $OUTPUT
md5sum $REFERENCE
md5sum $OUTPUT

