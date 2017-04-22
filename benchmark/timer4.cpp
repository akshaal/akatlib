#include <stdlib.h>
#include <avr/interrupt.h>
#include <avr/io.h>

#include "benchmark.h"

AKAT_DECLARE(/* cpu_frequency = */              8000000,
             /* tasks = */                      8,
             /* dispatcher_idle_code = */       ,
             /* dispatcher_overflow_code = */   )

AKAT_STIMER_8BIT (timer1, "r15") {
    BENCH
}

AKAT_STIMER_8BIT (timer2, "r14") {
    BENCH
}

AKAT_STIMER_8BIT (timer3, "r13") {
    BENCH
}

AKAT_STIMER_8BIT (timer4, "r12") {
    BENCH
}

void main () {
    akat_init ();

    BENCH_INIT

    BENCH
    timer1.set (2);

    BENCH
    timer2.set (1);

    BENCH
    timer3.set (2);

    BENCH
    timer4.set (1);

    BENCH
    akat_trigger_stimers (timer1, timer2, timer3, timer4);

    akat_trigger_stimers (timer1, timer2, timer3, timer4);

    akat_trigger_stimers (timer1, timer2, timer3, timer4);

    BENCH

    BENCH_EXIT
}
