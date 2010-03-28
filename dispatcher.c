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

#define MAX_TASKS 16
#define FIRST_SLOT (&tasks[0])
#define LAST_SLOT (&tasks[MAX_TASKS-1])

static volatile task_t tasks[MAX_TASKS];

static volatile task_t *free_slot = FIRST_SLOT;
static volatile task_t *filled_slot = FIRST_SLOT;
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

        task_t task_to_run;
        if (free_slot == filled_slot) {
            task_to_run = fixed_idle_task;
        } else {
            task_to_run = *filled_slot;
            if (filled_slot == LAST_SLOT) {
                filled_slot = FIRST_SLOT;
            } else {
                filled_slot ++;
            }
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
        volatile task_t *next_free_slot =
                   free_slot == LAST_SLOT
                           ? FIRST_SLOT
                           : (free_slot + 1);

        if (next_free_slot == filled_slot) {
            task_overflows ++;
            rc = 1;
        } else {
            *free_slot = task;
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
        volatile task_t *new_filled_slot =
                   filled_slot == FIRST_SLOT
                           ? LAST_SLOT
                           : (filled_slot - 1);

        if (new_filled_slot == free_slot) {
            task_overflows ++;
            rc = 1;
        } else {
            filled_slot = new_filled_slot;
            *filled_slot = task;
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
