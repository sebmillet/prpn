#!/bin/sh

PRG=../../src/prpn

if [ -z "$1" ]; then
	echo "Usage: $0 number"
	exit 1
fi

time $PRG -abz < input$1.txt
