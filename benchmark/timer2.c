#define F_CPU 1000000

#include <stdlib.h>
#include <avr/interrupt.h>
#include <avr/io.h>

#include "akat.h"
#include "benchmark.h"

AKAT_DECLARE(/* cpu_frequency = */     8000000,
             /* tasks = */             8,
             /* timers = */            2)

void task () {
}

void task2 () {
}

void main () {
    akat_init ();

    BENCH_INIT

    BENCH

    akat_schedule_stimer (task, 1);

    BENCH

    akat_schedule_stimer (task2, 2);

    BENCH

    akat_handle_stimers (); 

    BENCH

    akat_handle_stimers (); 

    BENCH

    akat_handle_stimers (); 

    BENCH

    BENCH_EXIT
}
