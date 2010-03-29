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
#include <stdlib.h>

#ifdef AKAT_DEBUG_ON
uint8_t is_akat_debug_on () {return 1;}
#else

#ifdef AKAT_DEBUG_OFF
uint8_t is_akat_debug_on () {return 0;}
#endif

#endif

/**
 * Initialize akat library.
 */
void akat_init ();

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Debug

/**
 * Like vprintf but output is redirected to the debug stream.
 */
void vdebugf (char *fmt, va_list ap);

/**
 * Like printf but output is redirected to the debug stream.
 */
void debugf (char *fmt, ...);

/**
 * Sends a string to output. This is a ligher and faster than debugf.
 */
void debug (char *str);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Dispatcher

typedef void (*task_t)(void);

/**
 * Dispatch tasks. When no tasks to dispatch, then run idle_task until there are tasks to dispatch.
 */
void akat_dispatcher_loop (task_t idle_task) __ATTR_NORETURN__;

/**
 * Dispatch task. Returns 1 if task was discarded (because tasks queue is full).
 */
uint8_t akat_dispatch (task_t task);

/**
 * Dispatch hi-priority task. Returns 1 if task was discarded (because tasks queue is full).
 */
uint8_t akat_dispatch_hi (task_t task);

/**
 * Returns number of overflows.
 */
uint8_t akat_dispatcher_overflows ();

#endif
