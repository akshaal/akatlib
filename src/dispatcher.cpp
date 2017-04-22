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

// This is defined by user to provide mask for tasks count
static uint8_t akat_dispatcher_tasks_mask() __ATTR_PURE__ __ATTR_CONST__;

// Defined by user to run code when dispatcher is idle.
static void akat_dispatcher_idle();

// Defined by user to handle a case when too many task are added to the dispatcher.
static void akat_dispatcher_overflow();

// This is supposed to be defined in the main file, not in library.
// Array of tasks.
extern volatile akat_task_t g_akat_tasks[];

// We use indexes, not pointers, because indexes are smaller (1 bytes) than pointers (2 bytes).
// Code is much smaller this way (version with pointer were evaluated).
register uint8_t g_free_slot asm("r4");;
register uint8_t g_filled_slot asm("r5");
register uint8_t g_slots asm("r6");

/**
 * Initialize disptacher.
 */
static void akat_init_dispatcher() {
    g_slots = akat_dispatcher_tasks_mask();
}

/**
 * Dispatch tasks.
 */
__ATTR_NORETURN__
static void akat_dispatcher_loop() {
    // Endless loop
    while (1) {
        // Select task to run
        cli();

        if (g_free_slot == g_filled_slot) {
            sei();
            akat_dispatcher_idle();
        } else {
            akat_task_t task_to_run = g_akat_tasks [g_filled_slot];

            AKAT_INC_REG (g_filled_slot);
            g_filled_slot &= g_slots;

            sei();
            task_to_run();
        }
    }
}

/**
 * Dispatch task. If tasks queue is full, then task is discarded!
 * Non atomic. Must be used with interrupts already disabled!
 */
static uint8_t akat_put_task_nonatomic(akat_task_t task) {
    uint8_t next_free_slot = (g_free_slot + 1) & g_slots;

    if (next_free_slot == g_filled_slot) {
        akat_dispatcher_overflow();
        return 1;
    } else {
        g_akat_tasks [g_free_slot] = task;
        g_free_slot = next_free_slot;
        return 0;
    }
}

/**
 * Dispatch task. If tasks queue is full, then task is discarded!
 */
static uint8_t akat_put_task(akat_task_t task) {
    uint8_t rc;

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        rc = akat_put_task_nonatomic(task);
    }

    return rc;
}

/**
 * Dispatch task. If tasks queue is full, then task is discarded!
 * Non atomic. Must be used with interrupts already disabled!
 */
static uint8_t akat_put_hi_task_nonatomic(akat_task_t task) {
    uint8_t new_filled_slot = (g_filled_slot - 1) & g_slots;

    if (new_filled_slot == g_free_slot) {
        akat_dispatcher_overflow();
        return 1;
    } else {
        g_filled_slot = new_filled_slot;
        g_akat_tasks [new_filled_slot] = task;
        return 0;
    }
}

/**
 * Dispatch task. If tasks queue is full, then task is discarded!
 */
static uint8_t akat_put_hi_task(akat_task_t task) {
    uint8_t rc;

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        rc = akat_put_hi_task_nonatomic(task);
    }

    return rc;
}
