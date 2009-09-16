## ROBOTLIB MAKEFILE ## Jorge Sanchez de Nova jssdn@kth.se ##

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

SOURCES = src/busio.c src/gpio.c
OBJECTS = $(SOURCES:.c=.o)
LIBNAME = librobot.a
DEBUG = -DDEBUGMODE

EXSOURCES = src/ex/robex.c
EXOBJECTS = $(EXSOURCES:.c=.o)
EXBIN = bin/robex
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

LDFLAGS=$(shell $(XENOCONFIG) --xeno-ldflags) $(MY_LDFLAGS) -O

# This includes the library path of given Xenomai into the binary to make live
# easier for beginners if Xenomai's libs are not in any default search path.
LDFLAGS+=-Xlinker -rpath -Xlinker $(shell $(XENOCONFIG) --libdir)

###### BUILD TARGETS ######
all:: robotlib examples

ifeq ($(BUILD_TYPE),DEBUG)
banner: 
	@echo  
	@echo -----------------------------------------------------------------------	
	@echo - Debug version of libraries: extra debugging output, no optimization 
	@echo - To create non-debug versions: make release                  
	@echo -----------------------------------------------------------------------
	@echo  

robotlib: banner $(SOURCES)
	$(CC) $(CFLAGS) $(LDFLAGS) $(CFLAGSDEB) -DDEBUGMODE $(INCLUDES) -c $(SOURCES)
	$(AR) lib/$(LIBNAME) $(OBJECTS)

examples: lib/$(LIBNAME)
	$(CC) $(CFLAGS) $(LDFLAGS) $(CFLAGSDEB) -DDEBUGMODE $(INCLUDES) $(EXSOURCES) $(LDBIN) -o $(EXBIN)
	
else
banner: 
	@echo  
	@echo -----------------------------------------------------------------------
	@echo - Release version of libraries: no debugging output.
	@echo - To create debug versions: make debug
	@echo -----------------------------------------------------------------------
	@echo  

robotlib: banner $(SOURCES)
	$(CC) $(CFLAGS) $(LDFLAGS) $(CFLAGSREL) $(INCLUDES) -c $(SOURCES)
	$(AR) lib/$(LIBNAME) $(OBJECTS)

examples: lib/$(LIBNAME)
	$(CC) $(CFLAGS) $(LDFLAGS) $(CFLAGSREL) $(INCLUDES) $(EXSOURCES) $(LDBIN) -o $(EXBIN)

endif

clean::
	$(RM) lib/*
	$(RM) src/*.o
	$(RM) bin/*

# mode_selection: 
# 	$(GCC) $(MODESEL_SOURCES) $(LDBIN) $(GCCFLAGSBIN) $(INCLUDES) -o $(MODESEL_BIN)
