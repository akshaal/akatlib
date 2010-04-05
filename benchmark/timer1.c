#define F_CPU 1000000

#include <stdlib.h>
#include <avr/interrupt.h>
#include <avr/io.h>

#include "akat.h"
#include "benchmark.h"

AKAT_INIT(/* timer_frequency = */   1000000,
          /* tasks = */             8,
          /* timers = */            1)

void task () {
}

void main () {
    BENCH_INIT

    BENCH

    akat_schedule_task (task, 1);

    BENCH

    akat_handle_timers (); 

    BENCH

    akat_handle_timers (); 

    BENCH

    BENCH_EXIT
}
