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

// Initializing declaration.
// freq - frequency
// tasks - maximum number of tasks in queue (allowed values: 1, 2, 4, 8, 16, 32, 64, 128).
// timers - maxium pending timers
#define AKAT_INIT(freq, tasks, timers) \
    volatile akat_task_t akat_tasks [tasks];                                    \
    volatile akat_timer_t akat_timers [timers] = {{0}};                         \
                                                                                \
    __ATTR_PURE__ __ATTR_CONST__ uint8_t akat_dispatcher_tasks_mask () {        \
        return tasks - 1;                                                       \
    }                                                                           \
                                                                                \
    __ATTR_PURE__ __ATTR_CONST__ uint8_t akat_timers_count () {                 \
        return timers;                                                          \
    }


// Declare debug flag functions by using one of definitions: AKAT_DUBUG_ON, AKAT_DEBUG_OFF.
// AKAT_DEBUG_ON takes precedence of AKAT_DEBUG_OFF. So if both are defined, then
// debug is considered ON. If none is defined, then debug state in unknown
// (program can't be linked).

#ifdef AKAT_DEBUG_ON
uint8_t is_akat_debug_on () {return 1;}
#else

#ifdef AKAT_DEBUG_OFF
uint8_t is_akat_debug_on () {return 0;}
#endif

#endif

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Debug

/**
 * Like vprintf but output is redirected to the debug stream.
 */
void akat_vdebugf (char *fmt, va_list ap);

/**
 * Like printf but output is redirected to the debug stream.
 */
void akat_debugf (char *fmt, ...);

/**
 * Sends a string to output. This is a ligher and faster than debugf.
 */
void akat_debug (char *str);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Dispatcher

typedef void (*akat_task_t)(void);

/**
 * Dispatch tasks. When no tasks to dispatch, then run idle_task until there are tasks to dispatch.
 */
void akat_dispatcher_loop (akat_task_t idle_task) __ATTR_NORETURN__;

/**
 * Dispatch task. Returns 1 if task was discarded (because tasks queue is full).
 * This function is supposed to be used only when interrupts are already disabled.
 */
uint8_t akat_put_task_nonatomic (akat_task_t task);

/**
 * Dispatch task. Returns 1 if task was discarded (because tasks queue is full).
 */
uint8_t akat_put_task (akat_task_t task);

/**
 * Dispatch hi-priority task. Returns 1 if task was discarded (because tasks queue is full).
 * This function is supposed to be used only when interrupts are already disabled.
 */
uint8_t akat_put_hi_task_nonatomic (akat_task_t task);

/**
 * Dispatch hi-priority task. Returns 1 if task was discarded (because tasks queue is full).
 */
uint8_t akat_put_hi_task (akat_task_t task);

/**
 * Returns number of overflows.
 */
uint8_t akat_dispatcher_overflows ();

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Dispatcher

typedef struct {
    akat_task_t task;
    uint16_t time;
    uint8_t hi;
} akat_timer_t;

/**
 * Handle timers. Should be called from a durable timer interrupt.
 */
void akat_handle_timers ();

/**
 * Schedule task for execution with hi priority.
 * If task is already scheduled, then task time is updated.
 * Returns 1 if task was discarded (because there are no free timer slots).
 *
 * This method should be called only with interrupts disabled.
 */
uint8_t akat_schedule_hi_task_nonatomic (akat_task_t new_task, uint16_t new_time);

/**
 * Schedule task for execution.
 * If task is already scheduled, then task time is updated.
 * Returns 1 if task was discarded (because there are no free timer slots).
 *
 * This method should be called only with interrupts disabled.
 */
uint8_t akat_schedule_task_nonatomic (akat_task_t new_task, uint16_t new_time);

/**
 * Schedule task for execution with hi priority.
 * If task is already scheduled, then task time is updated.
 * Returns 1 if task was discarded (because there are no free timer slots).
 */
uint8_t akat_schedule_hi_task (akat_task_t new_task, uint16_t new_time);

/**
 * Schedule task for execution.
 * If task is already scheduled, then task time is updated.
 * Returns 1 if task was discarded (because there are no fr1ee timer slots).
 */
uint8_t akat_schedule_task (akat_task_t new_task, uint16_t new_time);

/**
 * Returns number of overflows for timers.
 */
uint8_t akat_timers_overflows ();

#endif
