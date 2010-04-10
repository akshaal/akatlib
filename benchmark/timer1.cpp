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

void main () {
    akat_init ();

    BENCH_INIT

    BENCH_EXIT
}
