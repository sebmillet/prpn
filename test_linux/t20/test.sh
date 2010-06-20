#!/bin/sh

# t01/test.sh

# Tests with flag 31 set (default) => recall of last command arguments if an error occurs

../generic_simple.sh "Stack commands 1/3" "1-input.txt" "tmp-1-output.txt" "1-expected-output.txt" $1

# Tests with flag 31 cleared => no recall of last command arguments if an error occurs

../generic_simple.sh "Stack commands 2/3" "2-input.txt" "tmp-2-output.txt" "2-expected-output.txt" $1
