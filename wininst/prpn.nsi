; prpn.nsi
;
; This script is based on example2.nsi, provided with NSIS.
; It has uninstall support and (optionally) installs start menu shortcuts.
;
; It will install example2.nsi into a directory that the user selects,

;--------------------------------

Unicode True

; The name of the installer
Name "pRPN"

; The file to write
OutFile "pRPN-0.6.0-setup.exe"

; The default installation directory
InstallDir $PROGRAMFILES\pRPN

; Registry key to check for directory (so if you install again, it will
; overwrite the old one automatically)
InstallDirRegKey HKLM "Software\pRPN" "InstallationDirectory"

; Request application privileges for Windows Vista
RequestExecutionLevel user

;--------------------------------

; Pages

Page components
Page directory
Page instfiles

UninstPage uninstConfirm
UninstPage instfiles

; First is default
LoadLanguageFile "${NSISDIR}\Contrib\Language files\English.nlf"
LoadLanguageFile "${NSISDIR}\Contrib\Language files\French.nlf"

; Directly change the inner lang strings (Same as ComponentText)
LangString ^ComponentsText ${LANG_ENGLISH} "Setup of pRPN"
LangString ^ComponentsText ${LANG_FRENCH} "Configuration de l'installation de pRPN"

LangString Sec1 ${LANG_ENGLISH} "pRPN (required)"
LangString Sec1 ${LANG_FRENCH} "pRPN (obligatoire)"
LangString Sec2 ${LANG_ENGLISH} "Source files"
LangString Sec2 ${LANG_FRENCH} "Fichiers source"
LangString Sec3 ${LANG_ENGLISH} "Start Menu Shortcuts"
LangString Sec3 ${LANG_FRENCH} "Raccourcis menu"

; Country code, to be written in the registry
LangString LNCC ${LANG_ENGLISH} "en"
LangString LNCC ${LANG_FRENCH} "fr"

;--------------------------------

; The stuff to install
Section !$(Sec1)

	SectionIn RO

	; Set output path to the installation directory.
	SetOutPath $INSTDIR

	; Put file there
	File "prpn.exe"
	File "prpnc.exe"
	File "libgcc_s_seh-1.dll"
	File "libiconv-2.dll"
	File "libintl-8.dll"
	File "libjpeg-8.dll"
	File "liblzma-5.dll"
	File "libpng16-16.dll"
	File "libstdc++-6.dll"
	File "libtiff-5.dll"
	File "libwinpthread-1.dll"
	File "libzstd.dll"
	File "wxbase30u_gcc_custom.dll"
	File "wxmsw30u_core_gcc_custom.dll"
	File "wxmsw30u_html_gcc_custom.dll"
	File "zlib1.dll"

	StrCmp $(LNCC) "en" 0 +3
	File "pRPNen.html"
	File "pRPNen.txt"

	StrCmp $(LNCC) "fr" 0 +5
	File "pRPNfr.html"
	File "pRPNfr.txt"
	CreateDirectory $INSTDIR\$(LNCC)\LC_MESSAGES
	File "/oname=$(LNCC)\LC_MESSAGES\prpn.mo" "fr.mo"

	File "README.TXT"

	; Write the installation path into the registry
	WriteRegStr HKLM SOFTWARE\pRPN "InstallationDirectory" "$INSTDIR"
	WriteRegStr HKLM SOFTWARE\pRPN "Language" "$(LNCC)"

	; Write the uninstall keys for Windows
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\pRPN" "DisplayName" "pRPN"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\pRPN" "UninstallString" '"$INSTDIR\uninstall.exe"'
	WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\pRPN" "NoModify" 1
	WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\pRPN" "NoRepair" 1
	WriteUninstaller "uninstall.exe"

SectionEnd

Section !$(Sec2)

	SetOutPath $INSTDIR
	; Put file there
	File "prpn-0.6.0.tar.gz"

SectionEnd

; Optional section (can be disabled by the user)
Section !$(Sec3)

	CreateDirectory "$SMPROGRAMS\pRPN"
	CreateShortCut "$SMPROGRAMS\pRPN\pRPN.lnk" "$INSTDIR\prpn.exe" "" "$INSTDIR\prpn.exe" 0
	CreateShortCut "$SMPROGRAMS\pRPN\pRPN console.lnk" "$INSTDIR\prpnc.exe" "" "$INSTDIR\prpnc.exe" 0
	CreateShortCut "$SMPROGRAMS\pRPN\pRPN$(LNCC).html.lnk" "$INSTDIR\pRPN$(LNCC).html" "" "$INSTDIR\pRPN.html" 0
	CreateShortCut "$SMPROGRAMS\pRPN\README.TXT.lnk" "$INSTDIR\README.TXT" "" "$INSTDIR\README.TXT" 0
	CreateShortCut "$SMPROGRAMS\pRPN\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0

SectionEnd

;--------------------------------

; Uninstaller

Section "Uninstall"

	; Remove registry keys
	DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\pRPN"
	DeleteRegKey HKLM SOFTWARE\pRPN

	; Remove files and uninstaller
	Delete $INSTDIR\prpn.exe
	Delete $INSTDIR\prpnc.exe

	Delete $INSTDIR\libgcc_s_seh-1.dll
	Delete $INSTDIR\libiconv-2.dll
	Delete $INSTDIR\libintl-8.dll
	Delete $INSTDIR\libjpeg-8.dll
	Delete $INSTDIR\liblzma-5.dll
	Delete $INSTDIR\libpng16-16.dll
	Delete $INSTDIR\libstdc++-6.dll
	Delete $INSTDIR\libtiff-5.dll
	Delete $INSTDIR\libwinpthread-1.dll
	Delete $INSTDIR\libzstd.dll
	Delete $INSTDIR\wxbase30u_gcc_custom.dll
	Delete $INSTDIR\wxmsw30u_core_gcc_custom.dll
	Delete $INSTDIR\wxmsw30u_html_gcc_custom.dll
	Delete $INSTDIR\zlib1.dll

	; For previous installs
	Delete $INSTDIR\pRPN.html
	Delete $INSTDIR\pRPN.txt

	Delete $INSTDIR\pRPNen.html
	Delete $INSTDIR\pRPNen.txt

	Delete $INSTDIR\pRPNfr.html
	Delete $INSTDIR\pRPNfr.txt
	RMDir /r $INSTDIR\fr

	Delete $INSTDIR\README.TXT
	Delete $INSTDIR\prpn-0.6.0.tar.gz
	Delete $INSTDIR\uninstall.exe

	; Remove shortcuts, if any
	Delete "$SMPROGRAMS\pRPN\*.*"

	; Remove directories used
	RMDir "$SMPROGRAMS\pRPN"
	RMDir "$INSTDIR"

SectionEnd

;--------------------------------

Function .onInit

	;Language selection dialog

	Push ""
	Push ${LANG_ENGLISH}
	Push English
	Push ${LANG_FRENCH}
	Push French
	Push A ; A means auto count languages
	       ; for the auto count to work the first empty push (Push "") must remain
	LangDLL::LangDialog "Installer Language" "Please select the language of the installer"

	Pop $LANGUAGE
	StrCmp $LANGUAGE "cancel" 0 +2
		Abort
FunctionEnd

