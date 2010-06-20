#!/bin/sh

../generic_multiple.sh "-d t" 1 "Token parsing 1/2" "1-input" "tmp-output" "1-expected-output" ".txt" $1
../generic_multiple.sh "-d i" 1 "Token parsing 2/2" "2-input" "tmp-output" "2-expected-output" ".txt" $1
