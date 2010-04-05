#define F_CPU 1000000

#include <stdlib.h>
#include <avr/interrupt.h>
#include <avr/io.h>

#include "akat.h"
#include "benchmark.h"

AKAT_INIT(/* timer_frequency = */   1000000,
          /* tasks = */             8,
          /* timers = */            2)

void task () {
}

void task2 () {
}

void main () {
    BENCH_INIT

    BENCH

    akat_schedule_task (task, 1);

    BENCH

    akat_schedule_task (task2, 2);

    BENCH

    akat_handle_timers (); 

    BENCH

    akat_handle_timers (); 

    BENCH

    akat_handle_timers (); 

    BENCH

    BENCH_EXIT
}
