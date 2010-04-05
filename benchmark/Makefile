HOST = avr
CC = ${HOST}-gcc
OBJDUMP = ${HOST}-objdump
VERSION=1.0
CFLAGS=-std=gnu99 -Wall -fomit-frame-pointer \
        -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums \
        -fshort-enums  -Wl,--relax -ffreestanding \
        -fno-tree-scev-cprop -fno-strict-aliasing -mmcu=${MCU} \
        -flto -fwhole-program -Wl,-gc-sections -ffunction-sections \
        -fdata-sections -mcall-prologues

all: clean OsO3

distclean: clean

PARTS=tasks timer1 timer2 timer4

OsO3: ${patsubst %, O3-%.avr, ${PARTS}} ${patsubst %, Os-%.avr, ${PARTS}}
	echo "" >> result-${MCU}
	date >> result-${MCU}
	for name in `echo ${PARTS} | sed 's/ /\n/g'`; do \
          ./benchmark.py ${MCU} Os $${name}; \
          ./benchmark.py ${MCU} O3 $${name}; \
        done

Os-%.avr: %.c
	${CC} ${CFLAGS} -Os -I../ $< -DAKAT_DEBUG_OFF -save-temps -o $@ ../*.o
	${OBJDUMP} -d $@ > $@.s

O3-%.avr: %.c
	${CC} ${CFLAGS} -O3 -I../ $< -DAKAT_DEBUG_OFF -save-temps -o $@ ../*.o
	${OBJDUMP} -d $@ > $@.s

clean:
	rm -f *.o *.i *.s *.a *.avr
