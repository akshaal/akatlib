///////////////////////////////////////////////////////////////////
// Useful functions for rapid development for AVR microcontrollers.
// 2010 (C) Akshaal
// http://www.akshaal.info    or    http://rus.akshaal.info
// GNU GPL
///////////////////////////////////////////////////////////////////

// Timer service

#include <stdlib.h>
#include <avr/interrupt.h>
#include <util/atomic.h>

#include "akat.h"

#define TIMERS_MASK (akat_timers_mask ())

// This is defined by user
extern uint8_t akat_timers_count () __ATTR_PURE__ __ATTR_CONST__;

// This is supposed to be defined in the main file, not in library.
extern volatile akat_timer_t g_akat_timers[];

static volatile uint8_t g_timer_overflows = 0;

register uint8_t g_scheduled asm("r5");

/**
 * Initialize disptacher.
 */
void akat_init_timer () {
    g_scheduled = 0;   
}

/**
 * Check timers. Execute timers that are ready to be executed.
 * Must be called from interrupts only.
 */
void akat_handle_timers () {
    akat_timer_t *current_timer = (akat_timer_t*) g_akat_timers;

    uint8_t to_visit = g_scheduled;
    for (uint8_t i = akat_timers_count (); to_visit != 0 && --i != 255;) {
        uint16_t time = current_timer->time;
        if (time) {
            time--;
            to_visit--;
            current_timer->time = time;

            if (!time) {
                g_scheduled--;
                akat_task_t task = current_timer->task;
                current_timer->task = 0;

                task ();
            }
        }

        current_timer++;
    }
}

/**
 * Schedule task for execution.
 * This method should be called only with interrupts disabled.
 */
static uint8_t akat_schedule_task_nonatomic_with_prio (akat_task_t new_task,
                                                       uint16_t new_time)
{
    akat_timer_t *use_timer = 0;

    uint8_t to_visit = g_scheduled;
    akat_timer_t *current_timer = (akat_timer_t*) g_akat_timers;
    for (uint8_t i = akat_timers_count (); --i != 255; ) {
        akat_task_t current_task = current_timer->task;

        if (current_task) {
            if (current_task == new_task) {
                use_timer = current_timer;
                goto found;
            }
            to_visit--;
        } else {
            use_timer = current_timer;
            if (to_visit == 0) {
                goto found;
            }
        }

        current_timer++;
    }

    g_timer_overflows++;
    return 1;

found:
    use_timer->task = new_task;
    use_timer->time = new_time;
    g_scheduled++;

    return 0;
}

/**
 * Schedule task for execution.
 */
static uint8_t akat_schedule_task_with_prio (akat_task_t new_task, uint16_t new_time) {
    uint8_t rc;

    ATOMIC_BLOCK (ATOMIC_RESTORESTATE) {
        rc = akat_schedule_task_nonatomic_with_prio (new_task, new_time);
    }

    return rc;
}

/**
 * Schedule task for execution.
 * This method should be called only with interrupts disabled.
 */
uint8_t akat_schedule_task_nonatomic (akat_task_t new_task, uint16_t new_time) {
    return akat_schedule_task_nonatomic_with_prio (new_task, new_time);
}

/**
 * Schedule task for execution.
 */
uint8_t akat_schedule_task (akat_task_t new_task, uint16_t new_time) {
    return akat_schedule_task_with_prio (new_task, new_time);
}

/**
 * Returns number of overflows.
 */
uint8_t akat_timers_overflows () {
    return g_timer_overflows;
}
