## CAN Wrapper

This CAN Wrapper is written to adapt to the AMOS Flashbootloader Framework. It uses the Vector XL Driver library.


### Development
For development the following steps need to be done to do the correct linking of the library

Using Eclipse:
1. Go into Project Properties --> C/C++ Build/Settings
2. Under MinGW C++ Linker / Libraries --> Library Search Path (-L): Add the directory to where the .dll files are located for XL driver lib
   - Use the following path: "${workspace_loc:/${ProjName}/src/lib}"
3. Under MinGW C++ Linker / Libraries --> Libraries (-l): List all the dll files (do not include the .dll ending)
   - vxlapi
   - vxlapi64
4. Do Project --> Clean
5. Project --> Build All

### Application

To start the application:
1. Include the dll files next to the application executable

