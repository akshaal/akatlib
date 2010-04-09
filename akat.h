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

// Registered used by akat:
//    r3, r4 - dispatcher

#define FORCE_INLINE    __attribute__ ((always_inline))

// Initializing declaration.
// cpu_freq - timer frequency
// tasks - maximum number of tasks in queue (allowed values: 1, 2, 4, 8, 16, 32, 64, 128).
// stimers - maxium pending soft timers
#define AKAT_DECLARE(cpu_freq, tasks, stimers)                                         \
    volatile akat_task_t g_akat_tasks [tasks];                                         \
    volatile akat_stimer_t g_akat_stimers [stimers] = {{0}};                           \
                                                                                       \
    /* cpu freq */                                                                     \
    FORCE_INLINE uint32_t akat_cpu_freq_hz () {                                        \
        return cpu_freq;                                                               \
    }                                                                                  \
                                                                                       \
    /* tasks mask */                                                                   \
    __attribute__ ((error("Tasks count must be one of the following: 1,2,4,8,16,32"))) \
    extern void akat_dispatcher_error_ ();                                             \
                                                                                       \
    FORCE_INLINE uint8_t akat_dispatcher_tasks_mask () {                               \
        if (!(tasks == 1 || tasks == 2 || tasks == 4 || tasks == 8                     \
                           || tasks == 16 || tasks == 32))                             \
        {                                                                              \
            akat_dispatcher_error_ ();                                                 \
        }                                                                              \
        return tasks - 1;                                                              \
    }                                                                                  \
                                                                                       \
    /* soft timers count */                                                            \
    FORCE_INLINE uint8_t akat_stimers_count () {                                       \
        return stimers;                                                                \
    }


/**
 * Initialize akat library.
 */
void akat_init ();

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

// Returns cpu frequency HZ.
extern uint32_t akat_cpu_freq_hz ();

// Convert usecs to frequency
#define usecs2freq(usecs) (((uint32_t)1000000) / ((uint32_t)(usecs)))

// Convert frequency to usecs
#define freq2usecs(freq) (((uint32_t)1000000) / ((uint32_t)(freq)))

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
// Soft timers

typedef void (*akat_stimerf_t)(void);

typedef struct {
    akat_stimerf_t stimerf;
    uint16_t time;
} akat_stimer_t;

/**
 * Handle timers. Should be called from a durable timer interrupt.
 */
void akat_handle_stimers ();

/**
 * Schedule soft timer for execution.
 * If soft timer is already scheduled, then the soft timer's time is updated.
 * Returns 1 if the soft timer was discarded (because there are no free timer slots).
 *
 * This method should be called only with interrupts disabled.
 */
uint8_t akat_schedule_stimer_nonatomic (akat_stimerf_t new_stimerf, uint16_t new_time);

/**
 * Schedule soft timer for execution.
 * If soft timer is already scheduled, then soft timer's time is updated.
 * Returns 1 if the soft timer was discarded (because there are no free timer slots).
 */
uint8_t akat_schedule_stimer (akat_stimerf_t new_stimerf, uint16_t new_time);

/**
 * Returns number of overflows for timers.
 */
uint8_t akat_schedule_overflows ();

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// HW Timers

__attribute__((error("Unable to find prescaler")))
extern void akat_find_prescaler_error__ ();

__attribute__((error("Given timer interval can't be represented by prescaler and resolution.")))
extern void akat_find_prescaler_error_pr__ (uint32_t);

__attribute__((error("This function must be eliminated by GCC's optimizer")))
inline void akat_check_prescaler_and_resolution (uint32_t us,
                                                 uint16_t prescaler,
                                                 uint8_t resolution)
{
    uint32_t calculated_us = freq2usecs (akat_cpu_freq_hz () / prescaler / (resolution + 1));

    if (calculated_us != us) {
        akat_find_prescaler_error_pr__ (calculated_us);
    }
}

/*
 * Find prescaller value for the given frequency. Assuming that timer works
 * on the same clock as CPU.
 */
inline uint16_t akat_find_closest_prescaler (uint32_t us) {
    uint32_t freq_needed = usecs2freq (us);

    if (akat_cpu_freq_hz () / freq_needed < 256) {
        return 1;
    }

    if (akat_cpu_freq_hz () / 8 / freq_needed < 256) {
        return 8;
    }

    if (akat_cpu_freq_hz () / 64 / freq_needed < 256) {
        return 64;
    }

    if (akat_cpu_freq_hz () / 256 / freq_needed < 256) {
        return 256;
    }

    if (akat_cpu_freq_hz () / 1024 / freq_needed < 256) {
        return 1024;
    }

    akat_find_prescaler_error__ ();
    return 0;
}

/**
 * Find closest resolution for a timer that is supposed to be invoked every 'us' useconds
 * with the given 'prescaler'.
 */
inline uint16_t akat_find_closest_resolution (uint32_t us, uint16_t prescaler) {
    return akat_cpu_freq_hz() / usecs2freq (us) / prescaler - 1;
}

/**
 * Init timer0 in CTC mode.
 */
FORCE_INLINE
void akat_atmega16_internal_timer0_ctc (uint16_t prescaler, uint8_t resolution);

/**
 * Init timer0 in CTC mode.
 */
FORCE_INLINE
void akat_attiny85_internal_timer0_ctc (uint16_t prescaler,
                                        uint8_t resolutionA,
                                        uint8_t resolutionB);

/**
 * Init timer0 in CTC mode.
 */
FORCE_INLINE
void akat_atmega48_internal_timer0_ctc (uint16_t prescaler,
                                        uint8_t resolutionA,
                                        uint8_t resolutionB);

/**
 * Converts 'us' to prescaler and resolution and pass them as parameters to the given function.
 */
inline void akat_call_with_prescaler_and_resolution (uint32_t us,
                                                     void (*f)(uint16_t, uint8_t))
{
    uint16_t prescaler = akat_find_closest_prescaler (us);
    uint32_t resolution = akat_find_closest_resolution (us, prescaler);

    akat_check_prescaler_and_resolution (us, prescaler, resolution);
    f (prescaler, resolution);
}

/**
 * Converts 'us' to prescaler and resolution and pass them as parameters to the given function.
 * 'usA' or 'usB' might be 0 in order to disable the given channel.
 */
inline void akat_call_with_prescaler_and_resolutions (uint32_t usA,
                                                      uint32_t usB,
                                                      void (*f)(uint16_t, uint8_t, uint8_t))
{
    if (!usA && !usB) {
        return;
    }

    uint16_t prescalerA = usA ? akat_find_closest_prescaler (usA) : 0;
    uint32_t resolutionA = usA ? akat_find_closest_resolution (usA, prescalerA) : 0;

    uint16_t prescalerB = usB ? akat_find_closest_prescaler (usB) : 0;
    uint32_t resolutionB = usB ? akat_find_closest_resolution (usB, prescalerB) : 0;

    uint16_t prescaler = usA ? prescalerA : prescalerB;

    if (usA) {
        akat_check_prescaler_and_resolution (usA, prescaler, resolutionA);
    }

    if (usB) {
        akat_check_prescaler_and_resolution (usB, prescaler, resolutionB);
    }

    f (prescaler, resolutionA, resolutionB);
}

#endif
