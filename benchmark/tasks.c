#define F_CPU 1000000

#include <stdlib.h>
#include <avr/interrupt.h>
#include <avr/io.h>

#include "akat.h"
#include "benchmark.h"

AKAT_DECLARE(/* cpu_frequency = */     8000000,
             /* tasks = */             8,
             /* timers = */            1)

void idle () {
    BENCH_EXIT
}

void task () {
    BENCH
}

void task2 () {
    BENCH
}

void task_hi () {
    BENCH
}

void task2_hi () {
    BENCH
}

__ATTR_NORETURN__
void main () {
    akat_init ();

    BENCH_INIT

    BENCH

    akat_put_hi_task (task2_hi);

    BENCH

    akat_put_hi_task (task_hi);

    BENCH

    akat_put_task (task);

    BENCH

    akat_put_task (task2);

    BENCH

    akat_dispatcher_loop (idle);
}
