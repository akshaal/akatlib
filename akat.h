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
//    r3 - holds 1
//    r4, r5 - dispatcher

#define FORCE_INLINE    __attribute__ ((always_inline))

// Initializing declaration.
// cpu_freq - timer frequency
// tasks - maximum number of tasks in queue (allowed values: 1, 2, 4, 8, 16, 32, 64, 128).
// dispatcher_idle_code - code to run when dispatcher is idle
#define AKAT_DECLARE(cpu_freq,                                                         \
                     tasks,                                                            \
                     dispatcher_idle_code,                                             \
                     dispatcher_overflow_code)                                         \
    volatile akat_task_t g_akat_tasks [tasks];                                         \
                                                                                       \
    /* CPU freq */                                                                     \
    FORCE_INLINE uint32_t akat_cpu_freq_hz () {                                        \
        return cpu_freq;                                                               \
    }                                                                                  \
                                                                                       \
    /* Tasks count mask */                                                             \
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
    /* Code to run when dispatcher is idle. */                                         \
    FORCE_INLINE void akat_dispatcher_idle () {                                        \
        dispatcher_idle_code;                                                          \
    }                                                                                  \
                                                                                       \
    /* Code to run when dispatcher overflowed. */                                      \
    FORCE_INLINE void akat_dispatcher_overflow () {                                    \
        dispatcher_overflow_code;                                                      \
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

// Misc. Concatenate two names
#define AKAT_CONCAT(a, b)     a##b

// Fast increment/decrement
#define AKAT_INC_REG(reg) asm ("inc %0" : "+r" (reg));
#define AKAT_DEC_REG(reg) asm ("dec %0" : "+r" (reg));

// Register loaded with 1. +0 prevent from override
#define AKAT_ONE (akat_one__ + 0)

// This variable is supposed to host 1 always
register uint8_t akat_one__ asm("r3");

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
 * Dispatch tasks.
 */
void akat_dispatcher_loop () __ATTR_NORETURN__;

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

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Soft timers

/**
 * Trigger soft timers.
 */
template<typename Timer>
inline void akat_trigger_stimers (Timer &timer) {
    if (!timer.decrement_and_check ()) {
        timer.run ();
    }
};

/**
 * Trigger soft timers.
 */
template<typename Timer1, typename Timer2>
inline void akat_trigger_stimers (Timer1 &timer1, Timer2 &timer2) {
    // First step: Check all timers.

    uint8_t r1 = timer1.decrement_and_check ();
    uint8_t r2 = timer2.decrement_and_check ();

    // Second step: Run triggered tasks

    if (!r1) {
        timer1.run ();
    }

    if (!r2) {
        timer2.run ();
    }
};

/**
 * Trigger soft timers.
 */
template<typename Timer1, typename Timer2, typename Timer3>
inline void akat_trigger_stimers (Timer1 &timer1, Timer2 &timer2, Timer3 &timer3) {
    // First step: Check all timers.

    uint8_t r1 = timer1.decrement_and_check ();
    uint8_t r2 = timer2.decrement_and_check ();
    uint8_t r3 = timer3.decrement_and_check ();

    // Second step: Run triggered tasks

    if (!r1) {
        timer1.run ();
    }

    if (!r2) {
        timer2.run ();
    }

    if (!r3) {
        timer3.run ();
    }
};

/**
 * Trigger soft timers.
 */
template<typename Timer1, typename Timer2, typename Timer3, typename Timer4>
inline void akat_trigger_stimers (Timer1 &timer1, Timer2 &timer2, Timer3 &timer3, Timer4 &timer4) {
    // First step: Check all timers.

    uint8_t r1 = timer1.decrement_and_check ();
    uint8_t r2 = timer2.decrement_and_check ();
    uint8_t r3 = timer3.decrement_and_check ();
    uint8_t r4 = timer3.decrement_and_check ();

    // Second step: Run triggered tasks

    if (!r1) {
        timer1.run ();
    }

    if (!r2) {
        timer2.run ();
    }

    if (!r3) {
        timer3.run ();
    }

    if (!r4) {
        timer4.run ();
    }
};

#define AKAT_STIMER_8BIT(name, reg)                                           \
    register uint8_t __soft_timer_##name##_counter__ asm(reg);                \
                                                                              \
    FORCE_INLINE void __soft_timer_##name##_f__ ();                           \
                                                                              \
    struct name##_t {                                                         \
        FORCE_INLINE void set (uint8_t time) {                                \
            __soft_timer_##name##_counter__ = time;                           \
        }                                                                     \
                                                                              \
        FORCE_INLINE uint8_t get (uint8_t time) {                             \
            return __soft_timer_##name##_counter__;                           \
        }                                                                     \
                                                                              \
        FORCE_INLINE void cancel () {                                         \
            __soft_timer_##name##_counter__ = 0;                              \
        }                                                                     \
                                                                              \
        FORCE_INLINE uint8_t decrement_and_check () {                         \
            if (__soft_timer_##name##_counter__) {                            \
                AKAT_DEC_REG (__soft_timer_##name##_counter__);               \
                return __soft_timer_##name##_counter__;                       \
            }                                                                 \
            return 1;                                                         \
        }                                                                     \
                                                                              \
        FORCE_INLINE void run () {                                            \
            __soft_timer_##name##_f__ ();                                     \
        }                                                                     \
    } name;                                                                   \
                                                                              \
    FORCE_INLINE void __soft_timer_##name##_f__ ()


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// HW Timers

__attribute__((error("Unable to find prescaler")))
extern void akat_find_prescaler_error__ ();

__attribute__((error("Given timer interval can't be represented by prescaler and resolution.")))
extern void akat_find_prescaler_error_pr__ (uint32_t);

__attribute__((error("This function must be eliminated by GCC's optimizer")))
FORCE_INLINE inline void akat_check_prescaler_and_resolution (uint32_t us,
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
FORCE_INLINE inline uint16_t akat_find_closest_prescaler (uint32_t us) {
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
FORCE_INLINE inline uint16_t akat_find_closest_resolution (uint32_t us, uint16_t prescaler) {
    return akat_cpu_freq_hz() / usecs2freq (us) / prescaler - 1;
}

/**
 * Init timer0 in CTC mode.
 */
void akat_atmega16_internal_timer0_ctc (uint16_t prescaler, uint8_t resolution);

/**
 * Init timer0 in CTC mode.
 */
void akat_attiny85_internal_timer0_ctc (uint16_t prescaler,
                                        uint8_t resolutionA,
                                        uint8_t resolutionB);

/**
 * Init timer0 in CTC mode.
 */
void akat_atmega48_internal_timer0_ctc (uint16_t prescaler,
                                        uint8_t resolutionA,
                                        uint8_t resolutionB);

/**
 * Converts 'us' to prescaler and resolution and pass them as parameters to the given function.
 */
template<void (*f)(uint16_t, uint8_t)>
FORCE_INLINE inline void akat_call_with_prescaler_and_resolution (uint32_t us = 0)
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
template<void (*f)(uint16_t, uint8_t, uint8_t)>
FORCE_INLINE inline void akat_call_with_prescaler_and_resolutions (uint32_t usA = 0,
                                                                   uint32_t usB = 0)
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

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// GPIO

#define AKAT_DEFINE_PIN_ACCESS_FUNC(reg, port_char, pin_idx)         \
   FORCE_INLINE uint8_t is_##name () {                               \
       return AKAT_CONCAT(reg, port_char) & (1 << pin_idx);          \
   }                                                                 \
                                                                     \
   FORCE_INLINE void set_##name (uint8_t state) {                    \
       if (state) {                                                  \
           AKAT_CONCAT(reg, port_char) |= 1 << pin_idx;              \
       } else {                                                      \
           AKAT_CONCAT(reg, port_char) &= ~(1 << pin_idx);           \
       }                                                             \
   }

#define AKAT_DEFINE_PIN(name, port_char, pin_idx)                    \
    struct name##_t {                                                \
        AKAT_DEFINE_PIN_ACCESS_FUNC(port, PORT, port_char, pin_idx)  \
        AKAT_DEFINE_PIN_ACCESS_FUNC(ddr, DDR, port_char, pin_idx)    \
        AKAT_DEFINE_PIN_ACCESS_FUNC(pin, PIN, port_char, pin_idx)    \
    } name;

#endif
