wininst README.txt file
Copyright 2020 Sébastien Millet

HOW TO GENERATE WINDOWS INSTALLER
=================================

All steps below are done under Windows. Tested with Windows 10 1909.
It is assumed the host and the compilation target all are 64-bit architecture.
You may change it replacing x86_64 with i686 as appropriate.

0. About the tarball

   The tarbal archive (prpn-{version}.tar.gz) must be available in the source
   directory of prpn for the prep.sh script (see below) to be successful.
   Indeed, Windows installer proposes to install prpn source in addition to
   executables, therefore the tarball must be available.
   Today the file is:

      prpn-0.6.1.tar.gz

   and it is generated *FROM LINUX* executing make dist or make distcheck.

1. Download and install MSYS2

   It is available here:
     https://www.msys2.org

2. From MSYS2 command-line, install (pacman -S) the below packages:

   mingw-w64-x86_64-gcc
   mingw-w64-x86_64-wxWidgets
   mingw-w64-x86_64-make

   Note
       If you want to compile with wxWidgets 3.1 lib (3.1.3 as of writing of
       this document, November 2020), you have to install
         mingw-w64-x86_64-wxmsw3.1
       instead of
         mingw-w64-x86_64-wxWidgets

3. Install NSIS

   NSIS = Nullsoft Scriptable Install System. It is available here:
     https://sourceforge.net/projects/nsis/

4. Make sure your wxWidgets as seen by wx-config is the expected one

   The configure script (see below) will look for wxWidgets library, and use the
   default one. It'll simply run wx-config.
   You have to run
     wx-config --list
   to list the current (default) library default and alternatives. If the
   default one is not correct, you have to update /mingw64/bin/wx-config
   accordingly.

   The published Windows executable is generated with
   wxWidgets 3.1.3 non-static.

5. Compile prpn using mingw32-make, by executing

   cd /path/to/prpn-source
   ./configure MAKE=mingw32-make
   mingw32-make

   This'll produce 2 executable files:
      src/prpn.exe
      src/prpnc.exe

6. Prepare target files by executing:

   cd /path/to/prpn-source/wininst
   ./step1-prep.sh

   This is just a convenience script that'll copy reuqired DLLs and executables
   in the target directory.

7. (option a) Using Windows explorer, enter /path/to/prpn-source/wininst/winpkg
   directory, right-click on prpn.nsi and click the menu:

   Compile NSIS Script

7. (option b) Execute:

   ./step2-comp.sh

This'll produce a file named pRPN-{version}-setup.exe (where {version} is prpn
version), if it is version 0.6.1 (November 2020), it'll be:

   path/to/prpn-source/wininst/pRPN-0.6.1-setup.exe

You're done!

ABOUT PUBLISHED INSTALLER - NOVEMBER 2020
=========================================

   MSYS2 running on top of Windows 10 1909
   g++ --version: g++.exe (Rev4, Built by MSYS2 project) 10.2.0
   wx-config --list: 'Default config is msw-unicode-3.1'
     package: mingw-w64-x86_64-wxmsw3.1 3.1.3-1
   NSIS v3.06.1

