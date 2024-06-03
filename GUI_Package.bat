REM SPDX-License-Identifier: MIT
REM SPDX-FileCopyrightText: 2024 Wiktor Pilarczyk <wiktorpilar99@gmail.com>

set dir_path=%CD%
cd WINDOWS_GUI\build\Release
tar.exe -caf %dir_path%\GUI.zip *.dll WINDOWS_GUI.exe platforms