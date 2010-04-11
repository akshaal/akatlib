///////////////////////////////////////////////////////////////////
// Useful functions for rapid development for AVR microcontrollers.
// 2010 (C) Akshaal
// http://www.akshaal.info    or    http://rus.akshaal.info
// GNU GPL
///////////////////////////////////////////////////////////////////

// Initialization

#include <stdint.h>

#include "akat.h"

#include "debug.h"
#include "dispatcher.h"
#include "timer.h"

/**
 * Initialize akat library.
 */
void akat_init () {
    akat_one__  = 1;

    akat_init_debug ();
    akat_init_dispatcher ();
    akat_init_timer ();
}
