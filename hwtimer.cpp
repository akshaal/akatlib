///////////////////////////////////////////////////////////////////
// Useful functions for rapid development for AVR microcontrollers.
// 2010 (C) Akshaal
// http://www.akshaal.info    or    http://rus.akshaal.info
// GNU GPL
///////////////////////////////////////////////////////////////////

// HW Timers. Most used cases.

#include <avr/io.h>

#include "akat.h"

#define RESOLUTION(hz) ((hz) == 0 ? 0 : (akat_cpu_freq_hz () / hz))

FORCE_INLINE
static uint8_t calc_cs0_for_prescaler (uint16_t prescaler) {
    return prescaler == 1    ? ((0 << CS02) | (0 << CS01) | (1 << CS00)) :
           prescaler == 8    ? ((0 << CS02) | (1 << CS01) | (0 << CS00)) :
           prescaler == 64   ? ((0 << CS02) | (1 << CS01) | (1 << CS00)) :
           prescaler == 256  ? ((1 << CS02) | (0 << CS01) | (0 << CS00)) :
           prescaler == 1024 ? ((1 << CS02) | (0 << CS01) | (1 << CS00)) : 0;

}

FORCE_INLINE
void akat_atmega16_internal_timer0_ctc (uint16_t prescaler, uint8_t resolution) {
#if defined (__AVR_ATmega16__)
    OCR0 = resolution;
    TCCR0 = (1 << WGM01) | calc_cs0_for_prescaler (prescaler);
    TIMSK |= (1 << OCIE0);
#endif
}

FORCE_INLINE
void akat_attiny85_internal_timer0_ctc (uint16_t prescaler,
                                        uint8_t resolutionA,
                                        uint8_t resolutionB)
{
#if defined (__AVR_ATtiny85__)
    if (resolutionA != 0) {
        OCR0A = resolutionA;
    }

    if (resolutionB != 0) {
        OCR0B = resolutionB;
    }

    TCCR0A = (1 << WGM01);
    TCCR0B = calc_cs0_for_prescaler (prescaler);
    TIMSK |= ((resolutionA == 0) ? 0 : (1 << OCIE0A)) | ((resolutionB == 0) ? 0 : (1 << OCIE0B));
#endif
}

FORCE_INLINE
void akat_atmega48_internal_timer0_ctc (uint16_t prescaler,
                                        uint8_t resolutionA,
                                        uint8_t resolutionB)
{
#if defined (__AVR_ATmega48__)
    if (resolutionA != 0) {
        OCR0A = resolutionA;
    }

    if (resolutionB != 0) {
        OCR0B = resolutionB;
    }

    TCCR0A = (1 << WGM01);
    TCCR0B = calc_cs0_for_prescaler (prescaler);
    TIMSK0 |= ((resolutionA == 0) ? 0 : (1 << OCIE0A)) | ((resolutionB == 0) ? 0 : (1 << OCIE0B));
#endif
}
