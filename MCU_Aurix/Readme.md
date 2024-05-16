# MCU Aurix Development Studio Project

## Requirements

- install Aurix IDE ([Aurix Development Studio](https://softwaretools.infineon.com/tools?q=aurix))

## How to Run

### Import Project

- clone repository
- open Aurix IDE
- import project: `File > Import`
- Select project type `Infineon > Aurix Project`
- select local repositories and this folder `MCU_Aurix`
- Project names can not occur more than once. If a project with the same name needs to be imported, the old project must be deleted!

### Run/Flash

- build: white hammer button
- rebuild: blue hammer button
- flashing: orange chip button

### Debugging

- click on the bug symbol
- switch to the debug view (asks if it should happen automatically the first time, otherwise bug symbol in the top right corner)
- debug with resume, step over and into
- don't forget to terminate the debugging (red square in debug view) otherwise you cannot flash to the board

## Troubleshooting

- reinstall IDE, DAS and reboot
- open and close IDE and remove Devkit USB if IDE was running for a long time and errors occured
- `FTD2xx.dll can not be found` -> try with different USB cable
- `Could not load das_api.dll` and `DASWRAPPER:: ERROR init DAS API. Do you have DAS installed?`: Install older version of DAS (7.1.9) 