#!/usr/bin/bash

# Prepare Windows executables, help files etc., for NSIS (Nullsoft Scriptable
# Install System).

# Copyright 2020 Sébastien Millet

set -euo pipefail

rm -rf winpkg
mkdir winpkg

cp -i ../src/prpn.exe winpkg/
cp -i ../src/prpnc.exe winpkg/
cp -i prpn.nsi winpkg/
cp -i ../doc/pRPNen.html winpkg/
cp -i ../doc/pRPNen.txt winpkg/
cp -i ../doc/pRPNfr.html winpkg/
cp -i ../doc/pRPNfr.txt winpkg/
cp -i ../po/fr.gmo winpkg/fr.mo
cp -i ../README winpkg/README.TXT

cp -i dlls/libgcc_s_seh-1.dll winpkg/
cp -i dlls/libiconv-2.dll winpkg/
cp -i dlls/libintl-8.dll winpkg/
cp -i dlls/libjpeg-8.dll winpkg/
cp -i dlls/liblzma-5.dll winpkg/
cp -i dlls/libpng16-16.dll winpkg/
cp -i dlls/libstdc++-6.dll winpkg/
cp -i dlls/libtiff-5.dll winpkg/
cp -i dlls/libwinpthread-1.dll winpkg/
cp -i dlls/libzstd.dll winpkg/
cp -i dlls/wxbase30u_gcc_custom.dll winpkg/
cp -i dlls/wxmsw30u_core_gcc_custom.dll winpkg/
cp -i dlls/wxmsw30u_html_gcc_custom.dll winpkg/
cp -i dlls/zlib1.dll winpkg/

cp -i ../prpn-0.6.0.tar.gz winpkg/

