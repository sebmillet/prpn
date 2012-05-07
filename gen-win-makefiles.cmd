@REM
@REM gen-win-makefiles.cmd
@REM
@REM Update the two variables below according to your environment
@REM Sébastien Millet, 2010
@REM

@set DIR_INCLUDE_LIBINTL="C:\Progra~1\GnuWin32\include"
@set DIR_LIB_LIBINTL="C:\Progra~1\GnuWin32\lib"


@echo WxWidgets directory (WXWIN variable):     %WXWIN%
@echo gettext include path (to find libintl.h): %DIR_INCLUDE_LIBINTL%
@echo gettext lib path (to find libintl.lib):   %DIR_LIB_LIBINTL%
@pause

@set MYBKL=prpn.bkl
bakefile -f msvc %MYBKL% -I %WXWIN%\build\bakefiles\wxpresets -DMYINCLUDE=%DIR_INCLUDE_LIBINTL% -DMYLIB=%DIR_LIB_LIBINTL%
bakefile -f mingw %MYBKL% -I %WXWIN%\build\bakefiles\wxpresets -DMYINCLUDE=%DIR_INCLUDE_LIBINTL% -DMYLIB=%DIR_LIB_LIBINTL%

cd src

@set MYBKL=dev.bkl
bakefile -f mingw %MYBKL% -I %WXWIN%\build\bakefiles\wxpresets -DMYINCLUDE=%DIR_INCLUDE_LIBINTL% -DMYLIB=%DIR_LIB_LIBINTL%

cd ..
