#!/bin/sh

find -regex ".*/[tu][0-9][0-9]/tmp-[^.]+.txt$" | while read f; do rm "$f"; done
