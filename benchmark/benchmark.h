///////////////////////////////////////////////////////////////////
// Useful functions for rapid development for AVR microcontrollers.
// 2010 (C) Akshaal
// http://www.akshaal.info    or    http://rus.akshaal.info
// GNU GPL
///////////////////////////////////////////////////////////////////

#ifndef AKAT_BENCHMARK_H_
#define AKAT_BENCHMARK_H_

#define BENCH_INIT      DDRB = 0x3;

#define BENCH_EXIT      PORTB = 2;

#define BENCH           PORTB = 1; \
                        PORTB = 0;

#endif
