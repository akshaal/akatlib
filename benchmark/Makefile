AKAT_DIR=$(PWD)/$({0%/*})

include ${AKAT_DIR}/src/akat.Makefile

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

%.cpp.tmp.cpp: %.cpp ${AKAT_SRCS}
	cat ${AKAT_SRCS} "$<" > "$<.tmp.cpp"

Os-%.avr: %.cpp.tmp.cpp
	${CXX} ${CXXFLAGS} -Os "$<" -DAKAT_DEBUG_OFF -save-temps -o $@
	${OBJDUMP} -d $@ > $@.s

O3-%.avr: %.cpp.tmp.cpp
	${CXX} ${CXXFLAGS} -O3 "$<" -DAKAT_DEBUG_OFF -save-temps -o $@
	${OBJDUMP} -d $@ > $@.s

clean:
	rm -f *.ii *.o *.i *.s *.a *.avr *.tmp.cpp
