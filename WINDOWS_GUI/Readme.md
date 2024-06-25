# Build the Bootloader GUI

To build and run the GUI execute the corresponding bat script.

e.g.
```sh
./GUI_Build.bat
./GUI_Run.bat
```

Otherwise cmake can also be called directly
```
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --config Release
```