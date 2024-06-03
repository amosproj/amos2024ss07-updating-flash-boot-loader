@REM SPDX-License-Identifier: MIT
@REM SPDX-FileCopyrightText: 2024 Wiktor Pilarczyk <wiktorpilar99@gmail.com>

@echo off

IF EXIST "C:\Users\Public\Documents\Vector\XL Driver Library 20.30.14\bin" SET PATH=%PATH%;C:\Users\Public\Documents\Vector\XL Driver Library 20.30.14\bin

call "WINDOWS_GUI/build/Release/WINDOWS_GUI.exe"

REM mklink .\..\..\run_gui.lnk .\WINDOWS_GUI.exe

