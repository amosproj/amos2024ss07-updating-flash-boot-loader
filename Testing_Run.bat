REM SPDX-License-Identifier: MIT
REM SPDX-FileCopyrightText: 2024 Michael Bauer <mike.bauer@fau.de>

@echo off

IF EXIST "C:\Users\Public\Documents\Vector\XL Driver Library 20.30.14\bin" SET PATH=%PATH%;C:\Users\Public\Documents\Vector\XL Driver Library 20.30.14\bin

call "TESTING_WINDOWS_GUI/build/Release/TESTING_WINDOWS_GUI.exe"

REM mklink .\..\..\run_gui.lnk .\TESTING_WINDOWS_GUI.exe

