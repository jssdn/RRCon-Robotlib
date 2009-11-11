OROCOS_BUILD=/usr/local/ppc_4xx/build/
CC=powerpc-linux-g++
CXXFLAGS=-DOROCOS_TARGET=xenomai -I$(OROCOS_BUILD)/include -I../src
LDFLAGS=-L$(OROCOS_BUILD)lib -L../lib -lorocos-taskbrowser-xenomai -lorocos-ocl-common-xenomai \
-lorocos-rtt-xenomai -lpthread -lncurses -lnative -lrobot

testir: main.cpp ir.o
	${CC} main.cpp ir.o ${CXXFLAGS} ${LDFLAGS} -o testir

ir.o: ir.cpp ir.hpp
	${CC} ir.cpp ${CXXFLAGS} -c

clean:
	rm -f *~ 
	rm -f *.o 
	rm -f testir
