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

/**
 * Initialize akat library.
 */
void akat_init () {
    akat_init_debug ();
    akat_init_dispatcher ();
}
