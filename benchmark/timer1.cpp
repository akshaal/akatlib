#define F_CPU 1000000

#include <stdlib.h>
#include <avr/interrupt.h>
#include <avr/io.h>

#include "akat.h"
#include "benchmark.h"

AKAT_DECLARE(/* cpu_frequency = */              8000000,
             /* tasks = */                      8,
             /* dispatcher_idle_code = */       ,
             /* dispatcher_overflow_code = */   )

AKAT_STIMER_8BIT (timer1, "r15") {
    BENCH
}

void main () {
    akat_init ();

    BENCH_INIT

    BENCH
    timer1.set (2);

    BENCH
    akat_trigger_stimers (timer1);

    BENCH
    akat_trigger_stimers (timer1);

    akat_trigger_stimers (timer1);

    BENCH

    BENCH_EXIT
}
