set dir_path=%CD%
cd WINDOWS_GUI\build\Release
tar.exe -cf %dir_path%\GUI.zip *.dll WINDOWS_GUI.exe platforms