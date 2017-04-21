#define F_CPU 1000000

#include <stdlib.h>
#include <avr/interrupt.h>
#include <avr/io.h>

#include "akat.h"
#include "benchmark.h"

static void idle (void) {
    BENCH

    BENCH_EXIT
}

static void task (void) {
    BENCH
}

static void task2 (void) {
    BENCH
}

static void task_hi (void) {
    BENCH
}

static void task2_hi (void) {
    BENCH
}

AKAT_DECLARE(/* cpu_frequency = */              8000000,
             /* tasks = */                      8,
             /* dispatcher_idle_code = */       idle(),
             /* dispatcher_overflow_code = */   )

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

    akat_dispatcher_loop ();
}
