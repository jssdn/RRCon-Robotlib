# ROBOTLIB MAKEFILE 
GCC = gcc
GCCFLAGSDEB = -g -Wall 
GCCFLAGSREL = -Wall -O3 -mcpu=405

RM = rm -f
AR = ar rcs 
SOURCES = src/xspidev.c src/max1231adc.c src/i2ctools.c src/i2ctools/i2cbusses.c src/srf08.c src/lis3lv02dl.c src/tcn75.c src/hmc6352.c src/busio.c src/gpio.c src/lcd_proc.c src/openloop_motors.c src/hwservos.c

OBJECTS = xspidev.o max1231adc.o i2ctools.o i2cbusses.o srf08.o lis3lv02dl.o tcn75.o hmc6352.o busio.o gpio.o lcd_proc.o openloop_motors.o hwservos.o
LIBNAME = librobot.a
DEBUG = -DDEBUGMODE

EXSOURCES = src/ex/robex.c
EXBIN = bin/robex
IBIN = -Isrc/ -Isrc/i2ctools/
LDBIN = -Llib/ -lrobot
GCCFLAGSBIN = -g -Wall 

MODESEL_SOURCES = src/ex/mode_selection.c
MODESEL_BIN = bin/mode_selection

all: debug example_debug
	
release: $(SOURCES)
	@echo  
	@echo ----------------------------------------------------------------------
	@echo Release version of libraries: no debugging output.
	@echo To create debug versions: make debug
	@echo ----------------------------------------------------------------------
	@echo  
	$(GCC) $(GCCFLAGSREL) $(IBIN) -c $(SOURCES)
	$(AR) lib/$(LIBNAME) $(OBJECTS)

debug: $(SOURCES)
	@echo  
	@echo ----------------------------------------------------------------------	
	@echo Debug version of libraries: extra debugging output, no optimization
	@echo To create non-debug versions: make release
	@echo ----------------------------------------------------------------------
	@echo  
	
	$(GCC) $(GCCFLAGSDEB) $(IBIN) $(DEBUG) -c $(SOURCES)	
	$(AR) lib/$(LIBNAME) $(OBJECTS)
	
example_debug:
	$(GCC) $(EXSOURCES) $(LDBIN) $(GCCFLAGSBIN) $(IBIN) $(DEBUG) -o $(EXBIN)
	
example_release:
	$(GCC) $(EXSOURCES) $(LDBIN) $(GCCFLAGSBIN) $(IBIN) -o $(EXBIN)
	
mode_selection: 
	$(GCC) $(MODESEL_SOURCES) $(LDBIN) $(GCCFLAGSBIN) $(IBIN) -o $(MODESEL_BIN)

clean:
	$(RM) lib/*
	$(RM) *.o
	$(RM) bin/*
