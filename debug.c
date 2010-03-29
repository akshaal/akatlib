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

#include "debug.h"
#include "state.h"

static FILE debug_out;

/**
 * This external function should gives us state of debugging.
 */
extern uint8_t is_akat_debug_on () __ATTR_PURE__ __ATTR_CONST__;

/**
 * Send a char to the stream. Used to redirect debug output to the microcontroller's UART.
 */
static int akat_debug_uart_putchar (char c, FILE *stream) {
    // TODO: Fix bug here, this function must work correctly inside interrupts
#ifdef UCSR0A
    if (c == '\n') {
        akat_debug_uart_putchar('\r', stream);
    }

    loop_until_bit_is_set (UCSR0A, UDRE0);

    UDR0 = c;
#else
#ifdef UCSRA
    if (c == '\n') {
        akat_debug_uart_putchar('\r', stream);
    }

    loop_until_bit_is_set (UCSRA, UDRE);

    UDR = c;
#endif
#endif

    return 0;
}

/**
 * Init debug logger support.
 */
void akat_init_debug () {
    if (is_akat_debug_on ()) {
        fdev_setup_stream (&debug_out, akat_debug_uart_putchar, NULL, _FDEV_SETUP_WRITE);
    }
}

/**
 * Like vprintf but output is redirected to the debug stream.
 */
void akat_vdebugf (char *fmt, va_list ap) {
    if (is_akat_debug_on ()) {
        vfprintf (&debug_out, fmt, ap);
    }
}

/**
 * Like printf but output is redirected to the debug stream.
 */
void akat_debugf (char *fmt, ...) {
    if (is_akat_debug_on ()) {
        va_list ap;

        va_start (ap, fmt);
        akat_vdebugf (fmt, ap);
        va_end (ap);
    }
}

/**
 * Send string to the debug output.
 */
void akat_debug (char *str) {
    if (is_akat_debug_on ()) {
        fputs (str, &debug_out);
    }
}
