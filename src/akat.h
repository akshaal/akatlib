///////////////////////////////////////////////////////////////////
// Useful functions for rapid development for AVR microcontrollers.
// 2010 (C) Akshaal, Apache License
///////////////////////////////////////////////////////////////////

#ifndef AKAT_H_
#define AKAT_H_

#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>

// Registered used by akat:
//    r4, r5, r6 - dispatcher

#define FORCE_INLINE    __attribute__((always_inline)) inline
#define NO_INLINE       __attribute__((noinline))
#define __ATTR_UNUSED__       __attribute__((unused))

// Initializing declaration.
// cpu_freq - timer frequency
// tasks - maximum number of tasks in queue (allowed values: 1, 2, 4, 8, 16, 32, 64, 128).
// dispatcher_idle_code - code to run when dispatcher is idle
#define AKAT_DECLARE(cpu_freq,                                                         \
                     tasks,                                                            \
                     dispatcher_idle_code,                                             \
                     dispatcher_overflow_code)                                         \
    volatile akat_task_t g_akat_tasks[tasks];                                          \
                                                                                       \
    /* CPU freq */                                                                     \
    static FORCE_INLINE uint32_t akat_cpu_freq_hz()  {                                 \
        return cpu_freq;                                                               \
    }                                                                                  \
                                                                                       \
    /* Tasks count mask */                                                             \
    __attribute__ ((error("Tasks count must be one of the following: 1,2,4,8,16,32"))) \
    extern void akat_dispatcher_error_ ();                                             \
                                                                                       \
    static FORCE_INLINE uint8_t akat_dispatcher_tasks_mask () {                        \
        if (!(tasks == 1 || tasks == 2 || tasks == 4 || tasks == 8                     \
                           || tasks == 16 || tasks == 32))                             \
        {                                                                              \
            akat_dispatcher_error_ ();                                                 \
        }                                                                              \
        return tasks - 1;                                                              \
    }                                                                                  \
                                                                                       \
    /* Code to run when dispatcher is idle. */                                         \
    static FORCE_INLINE void akat_dispatcher_idle () {                                 \
        dispatcher_idle_code;                                                          \
        ;__asm__ ("\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n");                              \
    }                                                                                  \
                                                                                       \
    /* Code to run when dispatcher overflowed. */                                      \
    static FORCE_INLINE void akat_dispatcher_overflow () {                             \
        dispatcher_overflow_code;                                                      \
    }


/**
 * Initialize akat library.
 */
static void akat_init ();

// Declare debug flag functions by using one of definitions: AKAT_DUBUG_ON, AKAT_DEBUG_OFF.
// AKAT_DEBUG_ON takes precedence of AKAT_DEBUG_OFF. So if both are defined, then
// debug is considered ON. If none is defined, then debug state in unknown
// (program can't be linked).

#ifdef AKAT_DEBUG_ON
static FORCE_INLINE uint8_t is_akat_debug_on () {return 1;} __ATTR_CONST__ __ATTR_PURE__
#else
#ifdef AKAT_DEBUG_OFF
static FORCE_INLINE uint8_t is_akat_debug_on () {return 0;} __ATTR_CONST__ __ATTR_PURE__
#endif
#endif

// Returns cpu frequency HZ.
static uint32_t akat_cpu_freq_hz () __ATTR_CONST__ __ATTR_PURE__;

// Convert usecs to frequency
#define usecs2freq(usecs) (((uint32_t)1000000) / ((uint32_t)(usecs)))

// Convert frequency to usecs
#define freq2usecs(freq) (((uint32_t)1000000) / ((uint32_t)(freq)))

// Misc. Concatenate two names
#define AKAT_CONCAT(a, b)     a##b

// Fast increment/decrement
#define AKAT_INC_REG(reg) asm ("inc %0" : "+r" (reg));
#define AKAT_DEC_REG(reg) asm ("dec %0" : "+r" (reg));

// Delay. Delay function is non atomic!
// Routines are borrowed from avr-lib
__attribute__((error("akat_delay_us and akat_delay_us must be used with -O compiler flag and constant argument!")))
extern void akat_delay_us_error_nc__ ();

__attribute__((error("akat_delay_us and akat_delay_us can't perform such a small delay!")))
extern void akat_delay_us_error_delay__ ();

__attribute__((error("akat_delay_us and akat_delay_us can't perform such a long delay!")))
extern void akat_delay_us_error_bdelay__ ();

static FORCE_INLINE void akat_delay_us (uint32_t us) {
    if (!__builtin_constant_p(us)) {
        akat_delay_us_error_nc__ ();
    }

    uint64_t cycles = (uint64_t)us * (uint64_t)akat_cpu_freq_hz () / (uint64_t)1000000L;

    if (cycles / 3 == 0) {
        akat_delay_us_error_delay__ ();
    } else if (cycles / 3 < 256) {
        uint8_t __count = cycles / 3;

    __asm__ volatile (
        "1: dec %0" "\n\t"
        "brne 1b"
        : "=r" (__count)
        : "0" (__count)
    );
    } else if (cycles / 4 > 65535) {
        akat_delay_us_error_bdelay__ ();
    } else {
        uint16_t __count = cycles / 4;

    __asm__ volatile (
        "1: sbiw %0,1" "\n\t"
        "brne 1b"
        : "=w" (__count)
        : "0" (__count)
    );
    }
}

FORCE_INLINE void akat_delay_ms (uint16_t ms) {
    akat_delay_us (1000L * (uint32_t)ms);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Debug

/**
 * Like vprintf but output is redirected to the debug stream.
 */
static void akat_vdebugf(char *fmt, va_list ap) __ATTR_UNUSED__;

/**
 * Like printf but output is redirected to the debug stream.
 */
static void akat_debugf(char *fmt, ...) __ATTR_UNUSED__;

/**
 * Sends a string to output. This is a ligher and faster than debugf.
 */
static void akat_debug(char *str) __ATTR_UNUSED__;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Dispatcher

typedef void (*akat_task_t)(void);

/**
 * Dispatch tasks.
 */
static void akat_dispatcher_loop() __ATTR_NORETURN__;

/**
 * Dispatch task. Returns 1 if task was discarded (because tasks queue is full).
 * This function is supposed to be used only when interrupts are already disabled.
 */
static uint8_t akat_put_task_nonatomic(akat_task_t task) __ATTR_UNUSED__;

/**
 * Dispatch task. Returns 1 if task was discarded (because tasks queue is full).
 */
static uint8_t akat_put_task (akat_task_t task) __ATTR_UNUSED__;

/**
 * Dispatch hi-priority task. Returns 1 if task was discarded (because tasks queue is full).
 * This function is supposed to be used only when interrupts are already disabled.
 */
static uint8_t akat_put_hi_task_nonatomic (akat_task_t task) __ATTR_UNUSED__;

/**
 * Dispatch hi-priority task. Returns 1 if task was discarded (because tasks queue is full).
 */
static uint8_t akat_put_hi_task (akat_task_t task) __ATTR_UNUSED__;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Soft timers

/**
 * Trigger soft timers.
 */
template<typename Timer>
inline void akat_trigger_stimers(Timer &timer) {
    if (!timer.decrement_and_check ()) {
        timer.run ();
    }
};

/**
 * Trigger soft timers.
 */
template<typename Timer1, typename Timer2>
inline void akat_trigger_stimers(Timer1 &timer1, Timer2 &timer2) {
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
// GPIO

#define AKAT_DEFINE_PIN_ACCESS_FUNC(name, reg, port_char, pin_idx)   \
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
