REM prep.cmd
REM Prepare Windows installation
REM Highly depends on particular environment of the developer...

set VERSION=0.5.1
set SCP_EXE=C:\WINDOWS\pscp.exe
set PACKAGE_LOC=sebastien@maison-nblin://home/sebastien/travail/cpp/seb/prpn/prpn-%VERSION%.tar.gz

set MYREP=wininst

%SCP_EXE% %PACKAGE_LOC% .
cd ..
mingw32-make -f makefile.gcc
copy build\prpn.exe %MYREP%
copy build\prpnc.exe %MYREP%
copy doc\pRPNen.html %MYREP%
copy doc\pRPNen.txt %MYREP%
copy doc\pRPNfr.html %MYREP%
copy doc\pRPNfr.txt %MYREP%
copy po\fr.gmo %MYREP%\fr.mo
copy README %MYREP%\README.TXT

cd %MYREP%
