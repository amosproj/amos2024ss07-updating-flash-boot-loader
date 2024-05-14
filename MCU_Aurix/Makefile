
ifeq ($(OS),Windows_NT)
	SHELL := powershell.exe
else
	SHELL := powershell
endif
.SHELLFLAGS := -NoProfile -Command

# project name
PRJ = MCU_Aurix

EXT_C   = c
EXT_C++ = cpp

# directory names
SRCDIR := src
OBJDIR := obj
BINDIR := bin
INCDIR := inc

# program source files (not including external libraries)
SRC = $(wildcard main/*.c) # $(wildcard bootloader/src/*.c) $(wildcard driver/src/*.c)
# where to look for external libraries (consisting of .c/.cpp files and .h files)
# e.g. EXT = ../../EyeToSee ../../YouSART
# TODO add folders
EXT =

# include path
INCLUDE := #$(foreach dir, $(EXT), -I$(dir)) -Ibootloader/inc -Idriver/inc
# c flags
CFLAGS   = $(INCLUDE) -std=c99 -O0 -g3 -Wall -fmessage-length=0 -fno-common -fstrict-volatile-bitfields -fdata-sections -ffunction-sections -mtc162 #-MMD -MP
# TODO -Wall -Os

# any aditional flags for c++
CPPFLAGS =

# executables
# TODO remove paths
FLASHER = C:\Infineon\AURIX-Studio-1.9.20\tools\AurixFlasherSoftwareTool_v1.0.8\AurixFlasher.exe
OBJCOPY = C:\Infineon\AURIX-Studio-1.9.20\tools\Compilers\tricore-gcc11\bin\tricore-elf-objcopy.exe
OBJDUMP = C:\Infineon\AURIX-Studio-1.9.20\tools\Compilers\tricore-gcc11\bin\tricore-elf-objdump.exe
SIZE    = C:\Infineon\AURIX-Studio-1.9.20\tools\Compilers\tricore-gcc11\bin\tricore-elf-size.exe
CC      = C:\Infineon\AURIX-Studio-1.9.20\tools\Compilers\tricore-gcc11\bin\tricore-elf-gcc.exe

# tool configuration
TCPREFIX := C:\Infineon\AURIX-Studio-1.9.20\tools\Compilers\tricore-gcc11\bin\tricore-elf-
CC       := $(TCPREFIX)gcc.exe
++       := $(TCPREFIX)g++.exe
LD       := $(TCPREFIX)g++.exe
OBJDUMP  := $(TCPREFIX)objdump.exe
OBJCOPY  := $(TCPREFIX)objcopy.exe
SIZE     := $(TCPREFIX)size.exe

# generate list of objects
CFILES    = $(filter %.c, $(SRC))
EXTC     := $(foreach dir, $(EXT), $(wildcard $(dir)/*.c))
CPPFILES  = $(filter %.cpp, $(SRC))
EXTCPP   := $(foreach dir, $(EXT), $(wildcard $(dir)/*.cpp))
OBJFILES  = $(CFILES:.c=.o) $(EXTC:.c=.o) $(CPPFILES:.cpp=.o) $(EXTCPP:.cpp=.o)
OBJ 	  = $(addprefix $(OBJDIR)/, $(OBJFILES))
DEP 	  = $(OBJ:%.o=%.d)

# compile all files
# all: $(PRJ).hex

all: print obj/main/Cpu0_Main.o MCU_Aurix.elf

$(OBJDIR):
	@echo "MKDIR  $@"
	@mkdir $(OBJDIR)

print: | $(OBJDIR)
	@echo "$(SRC)"
	@echo "$(INCLUDE)"
	@echo "$(OBJ)"

flash: all
	$(FLASHER) -elf .\$(PRJ).elf

# remove compiled files
# TODO binaries in bin folder
clean:
	-@rm MCU_Aurix.hex
	-@rm MCU_Aurix.d
	-@rm MCU_Aurix.elf
	-@rm -r $(OBJDIR)
	-@rm -r $(BINDIR)
#TODO for reach
#	$(foreach dir, $(EXT), rm -f $(dir)/*.o;)

# compile C to object files
$(OBJDIR)/%.o: %.$(EXT_C) | $(OBJDIR)
	@echo "CC     $@"
	-@mkdir "$(@D)"
	@$(CC) $(CFLAGS) -c -o $@ $<

# elf file
$(PRJ).elf: $(OBJ)
#	@$(CC) $(CFLAGS) -o $(PRJ).elf $(OBJ)
	@$(CC) -nocrt0 -T"Lcf_Gnuc_Tricore_Tc.lsl" "-Wl,--gc-sections" -mtc162 -o "MCU_Aurix.elf" $(OBJ)

# # hex file
# $(PRJ).hex: $(PRJ).elf
# 	rm -f $(PRJ).hex
# 	$(OBJCOPY) -j .text -j .data -O ihex $(PRJ).elf $(PRJ).hex
# 	$(SIZE) $(PRJ).elf
