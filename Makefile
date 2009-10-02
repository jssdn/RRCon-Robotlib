## ROBOTLIB MAKEFILE ## Jorge Sanchez de Nova jssdn@kth.se ##
# NOTE: FOR CROSS COMPILING USE STH LIKE THIS: 
# > DESTDIR="/home/dilbert/xenomai/linux-2.6-denx/xenomai_userspace/local/" make LD_LIBRARY_PATH="/home/dilbert/xenomai/linux-2.6-denx/xenomai_userspace/local/usr/xenomai/lib" XENO="~/xenomai/linux-2.6-denx/xenomai_userspace/local/usr/xenomai/" ARCH="powerpc" KSRC="~/xenomai/linux-2.6-denx/" all
SHELL=/bin/bash
###### COMPILER CONFIGURATION ######
CROSS_COMPILE ?= 
ARCH ?= powerpc
BUILD_TYPE := DEBUG

CFLAGSDEB = -g -Wall 
CFLAGSREL = -Wall -O3 -mcpu=405

RM = rm -f
AR = $(CROSS_COMPILE)ar rcs 

###### ROBOTLIB CONFIGURATION ######

#SOURCES = src/xspidev.c src/max1231adc.c src/i2ctools.c src/i2ctools/i2cbusses.c src/srf08.c src/lis3lv02dl.c src/tcn75.c src/hmc6352.c src/busio.c src/gpio.c src/lcd_proc.c src/openloop_motors.c src/hwservos.c

SOURCES = src/busio.c src/gpio.c src/util.c src/platform_io.c src/motors.c src/xspidev.c src/max1231adc.c
# OBJECTS = $(SOURCES:.c=.o) # TODO:sed missing to remove src
OBJECTS = busio.o gpio.o util.o platform_io.o motors.o xspidev.o max1231adc.o
LIBNAME = librobot.a
DEBUG = -DDEBUGMODE

EXSOURCES = src/ex/platex.c
#EXSOURCES = xenomai_examples/control.c
EXOBJECTS = $(EXSOURCES:.c=.o)
EXBIN = bin/platex
INCLUDES = -Isrc/ -Isrc/i2ctools/
LDBIN = -Llib/ -lrobot
CFLAGSBIN = -g -Wall 

MODESEL_SOURCES = src/ex/mode_selection.c
MODESEL_BIN = bin/mode_selection

###### XENOMAI CONFIGURATION ######

### Note: to override the search path for the xeno-config script, use "make XENO=..."

###### USER SPACE BUILD ######

### Default Xenomai installation path
XENO ?= /usr/xenomai

XENOCONFIG=$(shell PATH=$(XENO):$(XENO)/bin:$(PATH) which xeno-config 2>/dev/null)

### Sanity check
ifeq ($(XENOCONFIG),)
all::
	@echo ">>> Invoke make like this: \"make XENO=/path/to/xeno-config\" <<<"
	@echo
endif


CC=$(shell $(XENOCONFIG) --cc)

CFLAGS=$(shell $(XENOCONFIG) --xeno-cflags) $(MY_CFLAGS)

LDFLAGS=$(shell $(XENOCONFIG) --xeno-ldflags) $(MY_LDFLAGS) -lnative -O

# This includes the library path of given Xenomai into the binary to make live
# easier for beginners if Xenomai's libs are not in any default search path.
LDFLAGS+=-Xlinker -rpath -Xlinker $(shell $(XENOCONFIG) --libdir)

###### BUILD TARGETS ######
all:: robotlib examples

ifeq ($(BUILD_TYPE),DEBUG)
banner: 
	@echo 
	@echo -e '\E[37;44m'"\033[1m-----------------------------------------------------------------------\033[0m"
	@echo -e '\E[37;39m'"-> Debug version of libraries: extra debugging output, no optimization\033[0m"
	@echo -e '\E[37;39m'"-> To create non-debug versions: append \033[1mBUILD_TYPE = RELEASE\033[0m"
	@echo -e '\E[37;44m'"\033[1m-----------------------------------------------------------------------\033[0m"
	@echo  

robotlib: banner $(SOURCES)
	@echo 
	@echo -e '\E[37;44m'"\033[1m----------------------------librobot------------------------------------\033[0m"
	$(CC) $(CFLAGS) $(LDFLAGS) $(CFLAGSDEB) -DDEBUGMODE $(INCLUDES) -c $(SOURCES)
	$(AR) lib/$(LIBNAME) $(OBJECTS)
	@echo -e '\E[37;44m'"\033[1m----------------------------Lib done!-----------------------------------\033[0m"
	@echo 
	
examples: lib/$(LIBNAME)	
	@echo 
	@echo -e '\E[37;44m'"\033[1m----------------------------examples-----------------------------------\033[0m"
	$(CC) $(CFLAGS) $(LDFLAGS) $(CFLAGSDEB) -DDEBUGMODE $(INCLUDES) $(EXSOURCES) $(LDBIN) -o $(EXBIN)
	@echo -e '\E[37;44m'"\033[1m----------------------------Examples done!-----------------------------\033[0m"
	@echo 
	
else
banner: 
	@echo  
	@echo -e '\E[37;31m'"\033[1m-----------------------------------------------------------------------\033[0m"
	@echo -e '\E[37;31m'"- Release version of libraries: no debugging output.\033[0m"
	@echo -e '\E[37;31m'"- To create debug versions: append \033[1mBUILD_TYPE = DEBUG\033[0m"
	@echo -e '\E[37;31m'"\033[1m-----------------------------------------------------------------------\033[0m"
	@echo  

robotlib: banner $(SOURCES)
	@echo 
	@echo -e '\E[37;31m'"\033[1m----------------------------librobot------------------------------------\033[0m"
	$(CC) $(CFLAGS) $(LDFLAGS) $(CFLAGSREL) $(INCLUDES) -c $(SOURCES)
	$(AR) lib/$(LIBNAME) $(OBJECTS)
	@echo -e '\E[37;31m'"\033[1m----------------------------Lib done!-----------------------------------\033[0m"
	@echo 

examples: lib/$(LIBNAME)
	@echo 
	@echo -e '\E[37;31m'"\033[1m----------------------------examples-----------------------------------\033[0m"
	$(CC) $(CFLAGS) $(LDFLAGS) $(CFLAGSREL) $(INCLUDES) $(EXSOURCES) $(LDBIN) -o $(EXBIN) 
	@echo -e '\E[37;31m'"\033[1m----------------------------Examples done!-----------------------------\033[0m"
	@echo 

endif

clean::
	$(RM) lib/*
	$(RM) src/*.o
	$(RM) bin/*

# mode_selection: 
# 	$(GCC) $(MODESEL_SOURCES) $(LDBIN) $(GCCFLAGSBIN) $(INCLUDES) -o $(MODESEL_BIN)
