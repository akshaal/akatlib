///////////////////////////////////////////////////////////////////
// Useful functions for rapid development for AVR microcontrollers.
// 2010 (C) Akshaal
// http://www.akshaal.info    or    http://rus.akshaal.info
// GNU GPL
///////////////////////////////////////////////////////////////////

// Debug

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>

static FILE g_debug_out;

/**
 * Send a char to the stream. Used to redirect debug output to the microcontroller's UART.
 */
static int akat_debug_uart_putchar(char c, FILE *stream) {
   ATOMIC_BLOCK (ATOMIC_RESTORESTATE) {
#ifdef UCSR0A
        if (c == '\n') {
            akat_debug_uart_putchar('\r', stream);
        }

        loop_until_bit_is_set(UCSR0A, UDRE0);

        UDR0 = c;
#else
#ifdef UCSRA
        if (c == '\n') {
            akat_debug_uart_putchar('\r', stream);
        }

        loop_until_bit_is_set(UCSRA, UDRE);

        UDR = c;
#endif
#endif
    }

    return 0;
}

/**
 * Init debug logger support.
 */
static void akat_init_debug() {
    if (is_akat_debug_on ()) {
        fdev_setup_stream (&g_debug_out, akat_debug_uart_putchar, NULL, _FDEV_SETUP_WRITE);
    }
}

/**
 * Like vprintf but output is redirected to the debug stream.
 */
static void akat_vdebugf(char *fmt, va_list ap) {
    if (is_akat_debug_on()) {
        vfprintf(&g_debug_out, fmt, ap);
    }
}

/**
 * Like printf but output is redirected to the debug stream.
 */
static void akat_debugf(char *fmt, ...) {
    if (is_akat_debug_on()) {
        va_list ap;

        va_start(ap, fmt);
        akat_vdebugf(fmt, ap);
        va_end(ap);
    }
}

/**
 * Send string to the debug output.
 */
static void akat_debug(char *str) {
    if (is_akat_debug_on()) {
        fputs(str, &g_debug_out);
    }
}
