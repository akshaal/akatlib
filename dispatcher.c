///////////////////////////////////////////////////////////////////
// Useful functions for rapid development for AVR microcontrollers.
// 2010 (C) Akshaal
// http://www.akshaal.info    or    http://rus.akshaal.info
// GNU GPL
///////////////////////////////////////////////////////////////////

// Dispatching

#include <stdlib.h>
#include <avr/interrupt.h>
#include <util/atomic.h>

#include "akat.h"

#define TASKS_MASK (akat_dispatcher_tasks_mask ())

// This is defined by user
extern uint8_t akat_dispatcher_tasks_mask () __ATTR_PURE__ __ATTR_CONST__;

// This is supposed to be defined in the main file, not in library.
extern volatile akat_task_t g_akat_tasks[];

// We use indexes, not pointers, because indexes are smaller (1 bytes) than pointers (2 bytes).
// Code is much smaller this way (version with pointer were evaluated).
register uint8_t g_free_slot asm("r3");
register uint8_t g_filled_slot asm("r4");
static volatile uint8_t g_task_overflows = 0;

/**
 * Initialize disptacher.
 */
void akat_init_dispatcher () {
    g_free_slot = 0;
    g_filled_slot = 0;
}

/**
 * Default idle task. Does nothing.
 */
static void akat_dispatcher_default_idle_task () {
}

/**
 * Dispatch tasks.
 */
__ATTR_NORETURN__
void akat_dispatcher_loop (akat_task_t idle_task) {
    akat_task_t fixed_idle_task = idle_task ?: akat_dispatcher_default_idle_task;

    // Endless loop
    while (1) {
        // Select task to run
        cli ();

        // Here we can use not volatile versions
        const uint8_t filled_slot_nv = (uint8_t) g_filled_slot;
        const uint8_t free_slot_nv = (uint8_t) g_free_slot;

        akat_task_t task_to_run;
        if (free_slot_nv == filled_slot_nv) {
            task_to_run = fixed_idle_task;
        } else {
            task_to_run = g_akat_tasks [filled_slot_nv];
            g_filled_slot = (filled_slot_nv + 1) & TASKS_MASK;
        }

        sei ();

        // Run task
        task_to_run ();
    }
}

/**
 * Dispatch task. If tasks queue is full, then task is discarded!
 * Non atomic. Must be used with interrupts already disabled!
 */
uint8_t akat_put_task_nonatomic (akat_task_t task) {
    uint8_t rc;

    // Here we can use not volatile versions
    const uint8_t filled_slot_nv = (uint8_t) g_filled_slot;
    const uint8_t free_slot_nv = (uint8_t) g_free_slot;

    const uint8_t next_free_slot = (free_slot_nv + 1) & TASKS_MASK;

    if (next_free_slot == filled_slot_nv) {
        g_task_overflows ++;
        rc = 1;
    } else {
        g_akat_tasks [free_slot_nv] = task;
        g_free_slot = next_free_slot;
        rc = 0;
    }

    return rc;
}

/**
 * Dispatch task. If tasks queue is full, then task is discarded!
 */
uint8_t akat_put_task (akat_task_t task) {
    uint8_t rc;

    ATOMIC_BLOCK (ATOMIC_RESTORESTATE) {
        rc = akat_put_task_nonatomic (task);
    }

    return rc;
}

/**
 * Dispatch task. If tasks queue is full, then task is discarded!
 * Non atomic. Must be used with interrupts already disabled!
 */
uint8_t akat_put_hi_task_nonatomic (akat_task_t task) {
    uint8_t rc;

    // Here we can use not volatile versions
    const uint8_t filled_slot_nv = (uint8_t) g_filled_slot;
    const uint8_t free_slot_nv = (uint8_t) g_free_slot;

    uint8_t new_filled_slot = (filled_slot_nv - 1) & TASKS_MASK;

    if (new_filled_slot == free_slot_nv) {
        g_task_overflows ++;
        rc = 1;
    } else {
        g_filled_slot = new_filled_slot;
        g_akat_tasks [new_filled_slot] = task;
        rc = 0;
    }

    return rc;
}

/**
 * Dispatch task. If tasks queue is full, then task is discarded!
 */
uint8_t akat_put_hi_task (akat_task_t task) {
    uint8_t rc;

    ATOMIC_BLOCK (ATOMIC_RESTORESTATE) {
        rc = akat_put_hi_task_nonatomic (task);
    }

    return rc;
}

/**
 * Returns number of overflows.
 */
uint8_t akat_dispatcher_overflows () {
    return g_task_overflows;
}
