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

#define TASKS_BITS 4
#define TASKS_MASK ((1 << TASKS_BITS) - 1)

static volatile task_t tasks [1 << TASKS_BITS];

// We use indexes, not pointers, because indexes are smaller (1 bytes) than pointers (2 bytes).
// Code is much smaller this way (version with pointer were evaluated).
static volatile uint8_t free_slot = 0;
static volatile uint8_t filled_slot = 0;
static volatile uint8_t task_overflows = 0;

/**
 * Initialize disptacher.
 */
void akat_init_dispatcher () {
}

/**
 * Default idle task. Does nothing.
 */
static void akat_default_idle_task () {
}

/**
 * Dispatch tasks.
 */
__ATTR_NORETURN__
void akat_dispatcher_loop (task_t idle_task) {
    task_t fixed_idle_task = idle_task ?: akat_default_idle_task;

    // Endless loop
    while (1) {
        // Select task to run
        cli ();

        // Here we can use not volatile versions
        const uint8_t filled_slot_nv = (uint8_t) filled_slot;
        const uint8_t free_slot_nv = (uint8_t) free_slot;

        task_t task_to_run;
        if (free_slot_nv == filled_slot_nv) {
            task_to_run = fixed_idle_task;
        } else {
            task_to_run = tasks [filled_slot_nv];
            filled_slot = (filled_slot_nv + 1) & TASKS_MASK;
        }

        sei ();

        // Run task
        task_to_run ();
    }
}

/**
 * Dispatch task. If tasks queue is full, then task is discarded!
 */
uint8_t akat_dispatch (task_t task) {
    uint8_t rc;

    ATOMIC_BLOCK (ATOMIC_RESTORESTATE) {
        // Here we can use not volatile versions
        const uint8_t filled_slot_nv = (uint8_t) filled_slot;
        const uint8_t free_slot_nv = (uint8_t) free_slot;

        const uint8_t next_free_slot = (free_slot_nv + 1) & TASKS_MASK;

        if (next_free_slot == filled_slot_nv) {
            task_overflows ++;
            rc = 1;
        } else {
            tasks [free_slot_nv] = task;
            free_slot = next_free_slot;
            rc = 0;
        }
    }

    return rc;
}

/**
 * Dispatch task. If tasks queue is full, then task is discarded!
 */
uint8_t akat_dispatch_hi (task_t task) {
    uint8_t rc;

    ATOMIC_BLOCK (ATOMIC_RESTORESTATE) {
        // Here we can use not volatile versions
        const uint8_t filled_slot_nv = (uint8_t) filled_slot;
        const uint8_t free_slot_nv = (uint8_t) free_slot;

        uint8_t new_filled_slot = (filled_slot_nv - 1) & TASKS_MASK;

        if (new_filled_slot == free_slot_nv) {
            task_overflows ++;
            rc = 1;
        } else {
            filled_slot = new_filled_slot;
            tasks [new_filled_slot] = task;
            rc = 0;
        }
    }

    return rc;
}

/**
 * Returns number of overflows.
 */
uint8_t akat_dispatcher_overflows () {
    return task_overflows;
}
