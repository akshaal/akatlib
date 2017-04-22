HOST = avr

CXX = ${HOST}-g++
OBJDUMP = ${HOST}-objdump

CXXFLAGS=-std=gnu++14 -Wall -Os -fomit-frame-pointer \
         -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums \
         -fshort-enums -ffreestanding -Wl,--relax \
         -fno-tree-scev-cprop -fno-strict-aliasing -mmcu=${MCU} \
         -mcall-prologues

AKAT_SRCS=${AKAT_DIR}/src/akat.h \
		  ${AKAT_DIR}/src/debug.cpp \
		  ${AKAT_DIR}/src/dispatcher.cpp \
		  ${AKAT_DIR}/src/init.cpp
