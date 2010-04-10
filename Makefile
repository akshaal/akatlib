# Note that gcc must be built with --enable-lto option (gcc 4.5 or newer)!
# At them momennt gold linker supports only x86 platform, so we are not going to use libraries

PREFIX=~/opt/avr/avr
HOST = avr
CXX = ${HOST}-g++
CXXFLAGS=-std=gnu++0x -Wall -Os -fomit-frame-pointer \
         -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums \
         -fshort-enums -ffreestanding -Wl,--relax \
         -fno-tree-scev-cprop -fno-strict-aliasing -mmcu=${MCU} \
         -flto -mcall-prologues 
VERSION=1.0

OBJS = dispatcher.o debug.o state.o init.o timer.o hwtimer.o
LIBNAME = libakat-${MCU}-${VERSION}

all: clean ${OBJS} benchmark

distclean: clean

benchmark: ${OBJS}
	mkdir -p out/${LIBNAME}/
	install -m 0644 ${OBJS} out/${LIBNAME}/
	+make -C benchmark

install: all
	rm -rf ${PREFIX}/lib/${LIBNAME}/
	mkdir -p ${PREFIX}/lib/${LIBNAME}/
	install -m 0644 ${OBJS} ${PREFIX}/lib/${LIBNAME}/
	install -m 0644 akat.h ${PREFIX}/include/akat-${VERSION}.h

clean:
	rm -rf *.o *.i *.s *.a out
	+make -C benchmark clean

.PHONY : install benchmark clean
