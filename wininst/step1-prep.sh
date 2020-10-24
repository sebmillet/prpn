#!/usr/bin/bash

# Prepare Windows executables, help files etc., for NSIS (Nullsoft Scriptable
# Install System).

# Copyright 2020 Sébastien Millet

set -euo pipefail

WP=winpkg

if [ -d "${WP}" ]; then
	echo -n "'${WP}' directory exists. Erase? (y/N) "
	read -r resp
	if [ "${resp}" != "y" ]; then
		echo "Aborted."
		exit
	fi
	rm -rf "${WP}"
fi

mkdir "${WP}"

cp -i ../src/prpn.exe "${WP}"/
cp -i ../src/prpnc.exe "${WP}"/
cp -i prpn.nsi "${WP}"/
cp -i ../doc/pRPNen.html "${WP}"/
cp -i ../doc/pRPNen.txt "${WP}"/
cp -i ../doc/pRPNfr.html "${WP}"/
cp -i ../doc/pRPNfr.txt "${WP}"/
cp -i ../po/fr.gmo "${WP}"/fr.mo
cp -i ../README "${WP}"/README.TXT

cp -i dlls/libgcc_s_seh-1.dll "${WP}"/
cp -i dlls/libiconv-2.dll "${WP}"/
cp -i dlls/libintl-8.dll "${WP}"/
cp -i dlls/libjpeg-8.dll "${WP}"/
cp -i dlls/liblzma-5.dll "${WP}"/
cp -i dlls/libpng16-16.dll "${WP}"/
cp -i dlls/libstdc++-6.dll "${WP}"/
cp -i dlls/libtiff-5.dll "${WP}"/
cp -i dlls/libwinpthread-1.dll "${WP}"/
cp -i dlls/libzstd.dll "${WP}"/
cp -i dlls/wxbase30u_gcc_custom.dll "${WP}"/
cp -i dlls/wxmsw30u_core_gcc_custom.dll "${WP}"/
cp -i dlls/wxmsw30u_html_gcc_custom.dll "${WP}"/
cp -i dlls/zlib1.dll "${WP}"/

cp -i ../prpn-0.6.0.tar.gz "${WP}"/

echo "'${WP}' ready, the next step is to execute step2-comp.sh"

