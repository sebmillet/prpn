REM prep.cmd
REM Prepare Windows installation
REM Highly depends on particular environment of the developer...

set VERSION=0.5.0
set SCP_EXE=C:\WINDOWS\pscp.exe
set PACKAGE_LOC=sebastien@maison-nblin://home/sebastien/travail/cpp/seb/prpn/prpn-%VERSION%.tar.gz

set MYREP=wininst

%SCP_EXE% %PACKAGE_LOC% .
cd ..
make -f makefile.bcc
copy build\prpn.exe %MYREP%
copy build\prpnc.exe %MYREP%
copy doc\pRPN.html %MYREP%
copy doc\pRPN.txt %MYREP%
copy README %MYREP%\README.TXT

cd %MYREP%
