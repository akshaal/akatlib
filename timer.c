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
extern volatile akat_timer_t akat_timers[];

static volatile uint8_t timer_overflows = 0;

/**
 * Initialize disptacher.
 */
void akat_init_timer () {
}

/**
 * Check timers. Execute timers that are ready to be executed.
 * Must be called from interrupts only.
 */
void akat_handle_timers () {
    akat_timer_t *akat_timers_nv = (akat_timer_t*) akat_timers;

    for (uint8_t i = akat_timers_count (); --i != 255; ) {
        akat_timer_t *current_timer = &akat_timers_nv[i];

        uint16_t time = current_timer->time;
        if (time) {
            time --;
            current_timer->time = time;

            if (!time) {
                akat_task_t task = current_timer->task;
                current_timer->task = 0;

                if (current_timer->hi) {
                    akat_put_hi_task_nonatomic (task);
                } else {
                    akat_put_task_nonatomic (task);
                }
            }
        }
    }
}

/**
 * Schedule task for execution.
 * This method should be called only with interrupts disabled.
 */
static uint8_t akat_schedule_task_nonatomic_with_prio (akat_task_t new_task,
                                                       uint16_t new_time,
                                                       uint8_t hi)
{
    akat_timer_t *akat_timers_nv = (akat_timer_t*) akat_timers;
    akat_timer_t *use_timer = 0;

    for (uint8_t i = akat_timers_count (); --i != 255; ) {
        akat_timer_t *current_timer = &akat_timers_nv[i];
        akat_task_t current_task = current_timer->task;

        if (current_task) {
            if (current_task == new_task) {
                use_timer = current_timer;
                break;
            }
        } else {
            use_timer = current_timer;
        }
    }

    uint8_t rc;
    if (use_timer) {
        use_timer->task = new_task;
        use_timer->time = new_time;
        use_timer->hi = hi;
        rc = 0;
    } else {
        timer_overflows += 1;
        rc = 1;
    }

    return rc;
}

/**
 * Schedule task for execution.
 */
static uint8_t akat_schedule_task_with_prio (akat_task_t new_task, uint16_t new_time, uint8_t hi) {
    uint8_t rc;

    ATOMIC_BLOCK (ATOMIC_RESTORESTATE) {
        rc = akat_schedule_task_nonatomic_with_prio (new_task, new_time, hi);
    }

    return rc;
}

/**
 * Schedule task for execution with hi priority.
 * This method should be called only with interrupts disabled.
 */
uint8_t akat_schedule_hi_task_nonatomic (akat_task_t new_task, uint16_t new_time) {
    return akat_schedule_task_nonatomic_with_prio (new_task, new_time, 1);
}

/**
 * Schedule task for execution.
 * This method should be called only with interrupts disabled.
 */
uint8_t akat_schedule_task_nonatomic (akat_task_t new_task, uint16_t new_time) {
    return akat_schedule_task_nonatomic_with_prio (new_task, new_time, 0);
}

/**
 * Schedule task for execution with hi priority.
 */
uint8_t akat_schedule_hi_task (akat_task_t new_task, uint16_t new_time) {
    return akat_schedule_task_with_prio (new_task, new_time, 1);
}

/**
 * Schedule task for execution.
 */
uint8_t akat_schedule_task (akat_task_t new_task, uint16_t new_time) {
    return akat_schedule_task_with_prio (new_task, new_time, 0);
}

/**
 * Returns number of overflows.
 */
uint8_t akat_timers_overflows () {
    return timer_overflows;
}
