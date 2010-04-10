HOST = avr
CXX = ${HOST}-g++
OBJDUMP = ${HOST}-objdump
VERSION=1.0
CXXFLAGS=-std=gnu++0x -Wall -fomit-frame-pointer \
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
          ./benchmark.py ${MCU} Os $${name} || true; \
          ./benchmark.py ${MCU} O3 $${name} || true; \
        done

Os-%.avr: %.cpp
	${CXX} ${CXXFLAGS} -Os -I../ $< -DAKAT_DEBUG_OFF -save-temps -o $@ ../*.o
	${OBJDUMP} -d $@ > $@.s

O3-%.avr: %.cpp
	${CXX} ${CXXFLAGS} -O3 -I../ $< -DAKAT_DEBUG_OFF -save-temps -o $@ ../*.o
	${OBJDUMP} -d $@ > $@.s

clean:
	rm -f *.o *.i *.s *.a *.avr
