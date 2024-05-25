set dir_path=%CD%
cd WINDOWS_GUI\build\Release
tar.exe -cf %dir_path%\GUI.tgz *.dll WINDOWS_GUI.exe platforms