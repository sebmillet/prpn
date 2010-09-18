#!/bin/sh

MYEDITOR="gvim -p"
cd src

$MYEDITOR Common.h Main.cpp Scalar.h Scalar.cpp Stack.h Stack.cpp Commands.h Variable.h Variable.cpp Parser.h Parser.cpp
$MYEDITOR UiImplWx.cpp UiImplConsole.cpp Ui.h Ui.cpp
$MYEDITOR platform/os_generic.h platform/os_generic.cpp platform/os_unknown.h platform/os_unix.h platform/os_win.h
