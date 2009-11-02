## ROBOTLIB MAKEFILE ## Jorge Sanchez de Nova jssdn@kth.se ##

# NOTE: FOR CROSS COMPILING USE STH LIKE THIS: 
# > DESTDIR="/home/dilbert/xenomai/linux-2.6-denx/xenomai_userspace/local/" make LD_LIBRARY_PATH="/home/dilbert/xenomai/linux-2.6-denx/xenomai_userspace/local/usr/xenomai/lib" XENO="~/xenomai/linux-2.6-denx/xenomai_userspace/local/usr/xenomai/" ARCH="powerpc" KSRC="~/xenomai/linux-2.6-denx/" all

SHELL=/bin/bash
###### COMPILER CONFIGURATION ######
CROSS_COMPILE ?= ppc_4xx-
ARCH ?= powerpc
BUILD_TYPE := DEBUG
KSRC ?= /usr/share/linux
ELDK ?= /opt/ELDK/4.2/ppc_4xx
CFLAGSDEB = -g -Wall 
CFLAGSREL = -Wall -O3 -mcpu=405

RM = rm -f
AR = $(CROSS_COMPILE)ar rcs 

###### ROBOTLIB CONFIGURATION ######

DEBUG = -DDEBUGALL
EXSOURCES = main.c marvell_88e1111.c mii-bitbang.c 
EXOBJECTS = $(EXSOURCES:.c=.o)
EXBIN = etherfix
INCLUDES = -I../src/ -I../src/i2ctools/ -I../include/
#-I$(KSRC)/include/ -I$(ELDK)/usr/include/
LDBIN = -L../lib/ -lrobot -L$(ELDK)/usr/lib -L$(ELDK)/lib
CFLAGSBIN = -g -Wall 

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
all:: etherfix

etherfix: clean $(SOURCES)
	@echo 
	@echo -e '\E[37;44m'"\033[1m----------------------------librobot-----------------------------------\033[0m"
	$(CC) $(CFLAGS) $(LDFLAGS) $(CFLAGSREL) $(DEBUG_WARN) $(INCLUDES) $(EXSOURCES) $(LDBIN) -o $(EXBIN) 
	@echo -e '\E[37;44m'"\033[1m----------------------------Lib done!----------------------------------\033[0m"
	@echo 

clean::
	$(RM) etherfix
