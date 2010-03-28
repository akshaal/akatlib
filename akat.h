///////////////////////////////////////////////////////////////////
// Useful functions for rapid development for AVR microcontrollers.
// 2010 (C) Akshaal
// http://www.akshaal.info    or    http://rus.akshaal.info
// GNU GPL
///////////////////////////////////////////////////////////////////

#ifndef AKAT_H_
#define AKAT_H_

#include <stdarg.h>
#include <stdint.h>

#ifdef AKAT_DEBUG_ON
short is_akat_debug_on () {return 1;}
#else

#ifdef AKAT_DEBUG_OFF
short is_akat_debug_on () {return 0;}
#endif

#endif

/**
 * Initialize akat library.
 */
void akat_init ();

/**
 * Like vprintf but output is redirected to the debug stream.
 */
void vdebugf (char *fmt, va_list ap);

/**
 * Like printf but output is redirected to the debug stream.
 */
void debugf (char *fmt, ...);

#endif
