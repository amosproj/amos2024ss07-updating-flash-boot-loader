@REM SPDX-License-Identifier: MIT
@REM SPDX-FileCopyrightText: 2024 Wiktor Pilarczyk <wiktorpilar99@gmail.com>
@REM SPDX-FileCopyrightText: 2024 Michael Bauer <mike.bauer@fau.de>

@echo off
set main_path=%CD%
rd /s /q build
mkdir build
cd build
set my_path=%CD%
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --config Release
call "C:\Qt\6.8.0\msvc2019_64\bin\qtenv2.bat"
cd /D %my_path%
cd Release
windeployqt.exe --quick --no-system-d3d-compiler --no-opengl-sw .

cd /D %main_path%

timeout /t 5