#! /bin/sh

BAKEFILE_PATHS=/usr/share/bakefile

bakefile -f gnu pRPN.bkl
mv GNUmakefile Makefile
cd src
bakefile -f gnu dev.bkl
mv GNUmakefile Makefile

cd ..

bakefile -f borland pRPN.bkl
cd src
bakefile -f borland dev.bkl
