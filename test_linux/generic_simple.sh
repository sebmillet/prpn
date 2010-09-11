#!/bin/sh

PRG=../../src/prpn

TNAME=$1
INPUT=$2
OUTPUT=$3
REFERENCE=$4

$PRG -abz < $INPUT > $OUTPUT

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

