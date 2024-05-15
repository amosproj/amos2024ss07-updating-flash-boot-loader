
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
SRC = $(wildcard main\\*.c) $(wildcard bootloader\\src\\*.c) $(wildcard driver\\src\\*.c) # TODO foreach SRCDIR
# where to look for external libraries (consisting of .c/.cpp files and .h files)
# e.g. EXT = ../../EyeToSee ../../YouSART
# TODO add folders
EXT = Configuration Configuration\\Debug Configurations Libraries Libraries\\Infra Libraries\\Infra\\Platform Libraries\\Infra\\Platform\\Tricore Libraries\\Infra\\Platform\\Tricore\\Compilers Libraries\\Infra\\Sfr Libraries\\Infra\\Sfr\\TC37A Libraries\\Infra\\Sfr\\TC37A\\_Reg Libraries\\Infra\\Ssw Libraries\\Infra\\Ssw\\TC37A Libraries\\Infra\\Ssw\\TC37A\\Tricore Libraries\\Service Libraries\\Service\\CpuGeneric Libraries\\Service\\CpuGeneric\\If Libraries\\Service\\CpuGeneric\\If\\Ccu6If Libraries\\Service\\CpuGeneric\\StdIf Libraries\\Service\\CpuGeneric\\SysSe Libraries\\Service\\CpuGeneric\\SysSe\\Bsp Libraries\\Service\\CpuGeneric\\SysSe\\Comm Libraries\\Service\\CpuGeneric\\SysSe\\General Libraries\\Service\\CpuGeneric\\SysSe\\Math Libraries\\Service\\CpuGeneric\\SysSe\\Time Libraries\\Service\\CpuGeneric\\_Utilities Libraries\\iLLD Libraries\\iLLD\\TC37A Libraries\\iLLD\\TC37A\\Tricore Libraries\\iLLD\\TC37A\\Tricore\\Asclin Libraries\\iLLD\\TC37A\\Tricore\\Asclin\\Asc Libraries\\iLLD\\TC37A\\Tricore\\Asclin\\Lin Libraries\\iLLD\\TC37A\\Tricore\\Asclin\\Spi Libraries\\iLLD\\TC37A\\Tricore\\Asclin\\Std Libraries\\iLLD\\TC37A\\Tricore\\Can Libraries\\iLLD\\TC37A\\Tricore\\Can\\Can Libraries\\iLLD\\TC37A\\Tricore\\Can\\Std Libraries\\iLLD\\TC37A\\Tricore\\Ccu6 Libraries\\iLLD\\TC37A\\Tricore\\Ccu6\\Icu Libraries\\iLLD\\TC37A\\Tricore\\Ccu6\\PwmBc Libraries\\iLLD\\TC37A\\Tricore\\Ccu6\\PwmHl Libraries\\iLLD\\TC37A\\Tricore\\Ccu6\\Std Libraries\\iLLD\\TC37A\\Tricore\\Ccu6\\TPwm Libraries\\iLLD\\TC37A\\Tricore\\Ccu6\\Timer Libraries\\iLLD\\TC37A\\Tricore\\Ccu6\\TimerWithTrigger Libraries\\iLLD\\TC37A\\Tricore\\Convctrl Libraries\\iLLD\\TC37A\\Tricore\\Convctrl\\Std Libraries\\iLLD\\TC37A\\Tricore\\Cpu Libraries\\iLLD\\TC37A\\Tricore\\Cpu\\Irq Libraries\\iLLD\\TC37A\\Tricore\\Cpu\\Std Libraries\\iLLD\\TC37A\\Tricore\\Cpu\\Trap Libraries\\iLLD\\TC37A\\Tricore\\Dma Libraries\\iLLD\\TC37A\\Tricore\\Dma\\Dma Libraries\\iLLD\\TC37A\\Tricore\\Dma\\Std Libraries\\iLLD\\TC37A\\Tricore\\Dts Libraries\\iLLD\\TC37A\\Tricore\\Dts\\Dts Libraries\\iLLD\\TC37A\\Tricore\\Dts\\Std Libraries\\iLLD\\TC37A\\Tricore\\Edsadc Libraries\\iLLD\\TC37A\\Tricore\\Edsadc\\Edsadc Libraries\\iLLD\\TC37A\\Tricore\\Edsadc\\Std Libraries\\iLLD\\TC37A\\Tricore\\Eray Libraries\\iLLD\\TC37A\\Tricore\\Eray\\Eray Libraries\\iLLD\\TC37A\\Tricore\\Eray\\Std Libraries\\iLLD\\TC37A\\Tricore\\Evadc Libraries\\iLLD\\TC37A\\Tricore\\Evadc\\Adc Libraries\\iLLD\\TC37A\\Tricore\\Evadc\\Std Libraries\\iLLD\\TC37A\\Tricore\\Fce Libraries\\iLLD\\TC37A\\Tricore\\Fce\\Crc Libraries\\iLLD\\TC37A\\Tricore\\Fce\\Std Libraries\\iLLD\\TC37A\\Tricore\\Flash Libraries\\iLLD\\TC37A\\Tricore\\Flash\\Std Libraries\\iLLD\\TC37A\\Tricore\\Geth Libraries\\iLLD\\TC37A\\Tricore\\Geth\\Eth Libraries\\iLLD\\TC37A\\Tricore\\Geth\\Std Libraries\\iLLD\\TC37A\\Tricore\\Gpt12 Libraries\\iLLD\\TC37A\\Tricore\\Gpt12\\IncrEnc Libraries\\iLLD\\TC37A\\Tricore\\Gpt12\\Std Libraries\\iLLD\\TC37A\\Tricore\\Gtm Libraries\\iLLD\\TC37A\\Tricore\\Gtm\\Atom Libraries\\iLLD\\TC37A\\Tricore\\Gtm\\Atom\\Dtm_PwmHl Libraries\\iLLD\\TC37A\\Tricore\\Gtm\\Atom\\Pwm Libraries\\iLLD\\TC37A\\Tricore\\Gtm\\Atom\\PwmHl Libraries\\iLLD\\TC37A\\Tricore\\Gtm\\Atom\\Timer Libraries\\iLLD\\TC37A\\Tricore\\Gtm\\Pwm Libraries\\iLLD\\TC37A\\Tricore\\Gtm\\Std Libraries\\iLLD\\TC37A\\Tricore\\Gtm\\Tim Libraries\\iLLD\\TC37A\\Tricore\\Gtm\\Tim\\In Libraries\\iLLD\\TC37A\\Tricore\\Gtm\\Tim\\Timer Libraries\\iLLD\\TC37A\\Tricore\\Gtm\\Tom Libraries\\iLLD\\TC37A\\Tricore\\Gtm\\Tom\\Dtm_PwmHl Libraries\\iLLD\\TC37A\\Tricore\\Gtm\\Tom\\Pwm Libraries\\iLLD\\TC37A\\Tricore\\Gtm\\Tom\\PwmHl Libraries\\iLLD\\TC37A\\Tricore\\Gtm\\Tom\\Timer Libraries\\iLLD\\TC37A\\Tricore\\Gtm\\Trig Libraries\\iLLD\\TC37A\\Tricore\\Hssl Libraries\\iLLD\\TC37A\\Tricore\\Hssl\\Hssl Libraries\\iLLD\\TC37A\\Tricore\\Hssl\\Std Libraries\\iLLD\\TC37A\\Tricore\\I2c Libraries\\iLLD\\TC37A\\Tricore\\I2c\\I2c Libraries\\iLLD\\TC37A\\Tricore\\I2c\\Std Libraries\\iLLD\\TC37A\\Tricore\\Iom Libraries\\iLLD\\TC37A\\Tricore\\Iom\\Driver Libraries\\iLLD\\TC37A\\Tricore\\Iom\\Iom Libraries\\iLLD\\TC37A\\Tricore\\Iom\\Std Libraries\\iLLD\\TC37A\\Tricore\\Msc Libraries\\iLLD\\TC37A\\Tricore\\Msc\\Msc Libraries\\iLLD\\TC37A\\Tricore\\Msc\\Std Libraries\\iLLD\\TC37A\\Tricore\\Mtu Libraries\\iLLD\\TC37A\\Tricore\\Mtu\\Std Libraries\\iLLD\\TC37A\\Tricore\\Pms Libraries\\iLLD\\TC37A\\Tricore\\Pms\\Std Libraries\\iLLD\\TC37A\\Tricore\\Port Libraries\\iLLD\\TC37A\\Tricore\\Port\\Io Libraries\\iLLD\\TC37A\\Tricore\\Port\\Std Libraries\\iLLD\\TC37A\\Tricore\\Psi5 Libraries\\iLLD\\TC37A\\Tricore\\Psi5\\Psi5 Libraries\\iLLD\\TC37A\\Tricore\\Psi5\\Std Libraries\\iLLD\\TC37A\\Tricore\\Psi5s Libraries\\iLLD\\TC37A\\Tricore\\Psi5s\\Psi5s Libraries\\iLLD\\TC37A\\Tricore\\Psi5s\\Std Libraries\\iLLD\\TC37A\\Tricore\\Qspi Libraries\\iLLD\\TC37A\\Tricore\\Qspi\\SpiMaster Libraries\\iLLD\\TC37A\\Tricore\\Qspi\\SpiSlave Libraries\\iLLD\\TC37A\\Tricore\\Qspi\\Std Libraries\\iLLD\\TC37A\\Tricore\\Scu Libraries\\iLLD\\TC37A\\Tricore\\Scu\\Std Libraries\\iLLD\\TC37A\\Tricore\\Sent Libraries\\iLLD\\TC37A\\Tricore\\Sent\\Sent Libraries\\iLLD\\TC37A\\Tricore\\Sent\\Std Libraries\\iLLD\\TC37A\\Tricore\\Smu Libraries\\iLLD\\TC37A\\Tricore\\Smu\\Smu Libraries\\iLLD\\TC37A\\Tricore\\Smu\\Std Libraries\\iLLD\\TC37A\\Tricore\\Src Libraries\\iLLD\\TC37A\\Tricore\\Src\\Std Libraries\\iLLD\\TC37A\\Tricore\\Stm Libraries\\iLLD\\TC37A\\Tricore\\Stm\\Std Libraries\\iLLD\\TC37A\\Tricore\\Stm\\Timer Libraries\\iLLD\\TC37A\\Tricore\\_Impl Libraries\\iLLD\\TC37A\\Tricore\\_Lib Libraries\\iLLD\\TC37A\\Tricore\\_Lib\\DataHandling Libraries\\iLLD\\TC37A\\Tricore\\_Lib\\InternalMux Libraries\\iLLD\\TC37A\\Tricore\\_PinMap bootloader bootloader\\inc driver driver\\inc

# include path
INCLUDE := $(foreach dir, $(EXT), -I$(dir)) #-Ibootloader/inc -Idriver/inc
# c flags
CFLAGS   = $(INCLUDE) -std=c99 -O0 -g3 -Wall -fmessage-length=0 -fno-common -fstrict-volatile-bitfields -fdata-sections -ffunction-sections -mtc162 #-MMD -MP
# TODO -Wall -Os

# any aditional flags for c++
CPPFLAGS =

# executables
# TODO remove paths
FLASHER = C:\\Infineon\\AURIX-Studio-1.9.20\\tools\\AurixFlasherSoftwareTool_v1.0.8\\AurixFlasher.exe
OBJCOPY = C:\\Infineon\\AURIX-Studio-1.9.20\\tools\\Compilers\\tricore-gcc11\\bin\\tricore-elf-objcopy.exe
OBJDUMP = C:\\Infineon\\AURIX-Studio-1.9.20\\tools\\Compilers\\tricore-gcc11\\bin\\tricore-elf-objdump.exe
SIZE    = C:\\Infineon\\AURIX-Studio-1.9.20\\tools\\Compilers\\tricore-gcc11\\bin\\tricore-elf-size.exe
CC      = C:\\Infineon\\AURIX-Studio-1.9.20\\tools\\Compilers\\tricore-gcc11\\bin\\tricore-elf-gcc.exe

# tool configuration
TCPREFIX := C:\\Infineon\\AURIX-Studio-1.9.20\\tools\\Compilers\\tricore-gcc11\\bin\\tricore-elf-
CC       := $(TCPREFIX)gcc.exe
++       := $(TCPREFIX)g++.exe
LD       := $(TCPREFIX)g++.exe
OBJDUMP  := $(TCPREFIX)objdump.exe
OBJCOPY  := $(TCPREFIX)objcopy.exe
SIZE     := $(TCPREFIX)size.exe

# generate list of objects
CFILES    = $(filter %.c, $(SRC))
EXTC     := $(foreach dir, $(EXT), $(wildcard $(dir)\\*.c))
CPPFILES  = $(filter %.cpp, $(SRC))
EXTCPP   := $(foreach dir, $(EXT), $(wildcard $(dir)\\*.cpp))
OBJFILES  = $(CFILES:.c=.o) $(EXTC:.c=.o) $(CPPFILES:.cpp=.o) $(EXTCPP:.cpp=.o)
OBJ 	  = $(addprefix $(OBJDIR)\\, $(OBJFILES))
DEP 	  = $(OBJ:%.o=%.d)

# compile all files
# all: $(PRJ).hex

all: MCU_Aurix.elf

$(OBJDIR):
	@echo "MKDIR  $@"
	mkdir $(OBJDIR)

print: | $(OBJDIR)
	@echo "$(SHELL)"
	@echo "SRC: $(SRC)"
	@echo "INCLUDE: $(INCLUDE)"
	@echo "OBJ: $(OBJ)"

flash: all
	$(FLASHER) -elf .\$(PRJ).elf

# remove compiled files
# TODO binaries in bin folder
clean:
	-rm MCU_Aurix.hex
	-rm MCU_Aurix.d
	-rm MCU_Aurix.elf
	-rm -r $(OBJDIR)
	-rm -r $(BINDIR)
#TODO for reach
#	$(foreach dir, $(EXT), rm -f $(dir)/*.o;)

# compile C to object files
$(OBJDIR)\\%.o: %.$(EXT_C) | $(OBJDIR)
	@echo "CC     $@"
	-mkdir -p "$(@D)"
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
