HOST = avr

CXX = ${HOST}-g++
OBJDUMP = ${HOST}-objdump

CXXFLAGS=-std=gnu++14 -Wall -fomit-frame-pointer \
         -funsigned-char -funsigned-bitfields -fpack-struct \
         -fshort-enums -ffreestanding -Wl,--relax -Wl,--gc-sections -fstack-arrays \
         -fno-strict-aliasing -mmcu=${MCU} -ftree-loop-ivcanon -fivopts -frename-registers \
         -fwhole-program -maccumulate-args -mcall-prologues -mrelax -nodevicelib -fipa-pta \
        --param early-inlining-insns=1000000 \
        --param max-hoist-depth=0 \
        --param max-tail-merge-comparisons=1000000 \
        --param max-tail-merge-iterations=1000000 \
        --param max-reload-search-insns=1000000 \
        --param max-cselib-memory-locations=100000 \
        --param max-sched-ready-insns=100000 \
        --param max-sched-region-blocks=1000 \
        --param max-pipeline-region-blocks=1000 \
        --param max-partial-antic-length=0 \
        --param sccvn-max-scc-size=1000000 \
        --param loop-invariant-max-bbs-in-loop=1000000 \
        --param loop-max-datarefs-for-datadeps=100000 \
        --param max-vartrack-size=0

AKAT_SRCS=${AKAT_DIR}/src/akat.h \
		  ${AKAT_DIR}/src/debug.cpp \
		  ${AKAT_DIR}/src/dispatcher.cpp \
		  ${AKAT_DIR}/src/init.cpp
