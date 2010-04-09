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

#define STIMERS_MASK (akat_stimers_mask ())

// This is defined by user
extern uint8_t akat_stimers_count () __ATTR_PURE__ __ATTR_CONST__;

// This is supposed to be defined in the main file, not in library.
extern volatile akat_stimer_t g_akat_stimers[];

static volatile uint8_t g_stimer_overflows = 0;

register uint8_t g_scheduled asm("r5");

/**
 * Initialize disptacher.
 */
void akat_init_timer () {
    g_scheduled = 0;   
}

/**
 * Check soft timers. Execute soft timers that are ready to be executed.
 * Must be called from interrupts only.
 */
void akat_handle_stimers () {
    akat_stimer_t *current_stimer = (akat_stimer_t*) g_akat_stimers;

    uint8_t to_visit = g_scheduled;
    for (uint8_t i = akat_stimers_count (); to_visit != 0 && --i != 255;) {
        uint16_t time = current_stimer->time;
        if (time) {
            time--;
            to_visit--;
            current_stimer->time = time;

            if (!time) {
                g_scheduled--;
                akat_stimerf_t stimerf = current_stimer->stimerf;
                current_stimer->stimerf = 0;

                stimerf ();
            }
        }

        current_stimer++;
    }
}

/**
 * Schedule soft timer for execution.
 * This method should be called only with interrupts disabled.
 */
uint8_t akat_schedule_stimer_nonatomic (akat_stimerf_t new_stimerf,
                                               uint16_t new_time)
{
    akat_stimer_t *use_stimer = 0;

    uint8_t to_visit = g_scheduled;
    akat_stimer_t *current_stimer = (akat_stimer_t*) g_akat_stimers;
    for (uint8_t i = akat_stimers_count (); --i != 255; ) {
        akat_stimerf_t current_stimerf = current_stimer->stimerf;

        if (current_stimerf) {
            if (current_stimerf == new_stimerf) {
                use_stimer = current_stimer;
                goto found;
            }
            to_visit--;
        } else {
            use_stimer = current_stimer;
            if (to_visit == 0) {
                goto found;
            }
        }

        current_stimer++;
    }

    g_stimer_overflows++;
    return 1;

found:
    use_stimer->stimerf = new_stimerf;
    use_stimer->time = new_time;
    g_scheduled++;

    return 0;
}

/**
 * Schedule soft timer for execution.
 */
uint8_t akat_schedule_stimer (akat_stimerf_t new_stimer, uint16_t new_time) {
    ATOMIC_BLOCK (ATOMIC_RESTORESTATE) {
        return akat_schedule_stimer_nonatomic (new_stimer, new_time);
    }
}

/**
 * Returns number of overflows.
 */
uint8_t akat_schedule_overflows () {
    return g_stimer_overflows;
}
