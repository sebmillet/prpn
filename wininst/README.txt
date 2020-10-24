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

      prpn-0.6.0.tar.gz

   and it is generated *FROM LINUX* executing make dist or make distcheck.

1. Download and install MSYS2

   It is available here:
     https://www.msys2.org

2. From MSYS2 command-line, install (pacman -S) the below packages:

   mingw-w64-x86_64-gcc
   mingw-w64-x86_64-wxWidgets
   mingw-w64-x86_64-make

3. Install NSIS

   NSIS = Nullsoft Scriptable Install System. It is available here:
     https://sourceforge.net/projects/nsis/

4. Compile prpn using mingw32-make, by executing

   cd /path/to/prpn-source
   ./configure MAKE=mingw32-make
   mingw32-make

   This'll produce 2 executable files:
      src/prpn.exe
      src/prpnc.exe

5. Prepare target files by executing:

   cd /path/to/prpn-source/wininst
   ./step1-prep.sh

6. (option a) Using Windows explorer, enter /path/to/prpn-source/wininst/winpkg
   directory, right-click on prpn.nsi and click the menu:

   Compile NSIS Script

6. (option b) Execute:

   ./step2-comp.sh

This'll produce a file named pRPN-{version}-setup.exe (where {version} is prpn
version), now it is:

   pRPN-0.6.0-setup.exe

You're done!

