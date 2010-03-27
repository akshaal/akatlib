# Note that gcc must be built with --enable-lto option (gcc 4.5 or newer)!
# At them momennt gold linker supports only x86 platform, so we are not going to use libraries

PREFIX=~/opt/avr
HOST = avr
CC = ${HOST}-gcc
CFLAGS=-std=gnu99 -Wall -Os -fomit-frame-pointer \
       -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums \
       -fshort-enums -ffreestanding -Wl,--relax \
       -fno-tree-scev-cprop -fno-strict-aliasing -mmcu=${MCU} \
       -flto
VERSION=1.0

OBJS = dispatcher.o
LIBNAME = libakatlib-${MCU}-${VERSION}

all: ${OBJS}

distclean: clean

install: all
	rm -rf ${PREFIX}/lib/${LIBNAME}/
	mkdir -p ${PREFIX}/lib/${LIBNAME}/
	install -m 0644 ${OBJS} ${PREFIX}/lib/${LIBNAME}/
	install -m 0644 akatlib.h ${PREFIX}/include/akatlib-${VERSION}.h

clean:
	rm -f *.o *.i *.s *.a
