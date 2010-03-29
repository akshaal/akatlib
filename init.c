///////////////////////////////////////////////////////////////////
// Useful functions for rapid development for AVR microcontrollers.
// 2010 (C) Akshaal
// http://www.akshaal.info    or    http://rus.akshaal.info
// GNU GPL
///////////////////////////////////////////////////////////////////

// Initialization

#include <stdint.h>

#include "debug.h"
#include "dispatcher.h"
#include "timer.h"

/**
 * Initialize akat library. Always returns 0.
 */
 __attribute__ ((constructor))
uint8_t akat_init_ () {
    akat_init_debug ();
    akat_init_dispatcher ();
    akat_init_timer ();

    return 0;
}
