///////////////////////////////////////////////////////////////////
// Useful functions for rapid development for AVR microcontrollers.
// 2010 (C) Akshaal
// http://www.akshaal.info    or    http://rus.akshaal.info
// GNU GPL
///////////////////////////////////////////////////////////////////

#ifndef AKAT_ONEWIRE_H_
#define AKAT_ONEWIRE_H_

#ifndef AKAT_H_
#  error "Include akat.h before including akat-onewire.h"
#endif

#include <avr/eeprom.h>

/**
 * Onewire_stage.
 */
typedef enum {AKAT_ONEWIRE_IDLE,
              AKAT_ONEWIRE_WAITING_RESET_MINIMUM,
              AKAT_ONEWIRE_WAITING_RESET_MAXIMUM,
              AKAT_ONEWIRE_PRESENCE_PULSE_ON,
              AKAT_ONEWIRE_WAITING_FIRST_SIGNAL}
        akat_onewire_stage_t;

/**
 * Flags used by onewire.
 */
struct akat_onewire_flags_t {
    uint8_t ignore_one_change : 1;
    uint8_t alarm : 1;
    uint8_t error : 1;
    uint8_t slot_started : 1;
};

/**
 * Calcalate crc8.
 */
uint8_t akat_onewire_crc8 (uint8_t data, uint8_t crc) {
    for (uint8_t i = 8; i > 0; i--) {
        uint8_t bit = (crc ^ data) & 0x01;

        crc = crc >> 1;

        if (bit) {
            crc = crc ^ ((0x18 >> 1) | 0x80);
        }

        data = data >> 1;
    }

    return crc;
}

#define AKAT_DEFINE_ONEWIRE_ID(name, family, id1, id2, id3, id4, id5, id6)  \
uint8_t EEMEM name[] = {family, id1, id2, id3, id4, id5, id6,               \
            akat_onewire_crc<                                               \
                akat_onewire_crc<                                           \
                    akat_onewire_crc<                                       \
                        akat_onewire_crc<                                   \
                            akat_onewire_crc<                               \
                                akat_onewire_crc<                           \
                                        akat_onewire_crc<0, family>::value, \
                                        id1>::value,                        \
                                    id2>::value,                            \
                                id3>::value,                                \
                            id4>::value,                                    \
                        id5>::value,                                        \
                    id6>::value};

/*
 * Main onewire structure.
 */
template<typename DQ_T,
         DQ_T &dq,
         typename STIMER,
         STIMER &stimer,
         uint8_t timer_us,
         typename HELPERS,
         HELPERS &helpers,
         uint8_t *id,
         void (*handle_function)()>
struct akat_onewire_t {
    private:

    // Constants
    enum {
        // Minimum time for RESET pulse to stay LOW. Soft timer is used.
        RX_RESET_MIN_US                     = 480 - timer_us * 2,

        // Maximum time for RESET pulse to stay LOW. Soft timer is used.
        RX_RESET_MAX_US                     = 960 + timer_us * 2,

        // Time needed before we respond to master
        DELAY_BEFORE_PRESENCE_US            = 15,

        // Time to hold line LOW when generating PRESENCE pulse. Soft timer is used
        TX_PRESENCE_US                      = 120 + timer_us * 2,
        TX_PRESENCE_MAX_US                  = 240,

        // Sample value from master after this number of microseconds elapsed
        // from the start of the slot.
        RX_SLOT_SAMPLING_OFFSET_US          = 30,

        // In this number of microseconds master is supposed to settle on line
        RX_SLOT_MIN_US                      = 60,

        // Maximum number of microseconds one slot can occupy
        RX_SLOT_MAX_US                      = 120,

        // How many microseconds we can tolerate if AVR/GCC makes a wrong delay
        AVR_DELAYS_ERROR_US                 = 4,

        // We will wait no more than this amount of microseconds
        // for a NEXT slot signal to arrive from master
        RX_SLOT_SIGNAL_WAITING_US           = 20000, // 20000 us

        // Maximum time we wait for the first signal to arrive after the present pulse
        RX_FIRST_SIGNAL_MAX_100US           = RX_SLOT_SIGNAL_WAITING_US  / 100,

        // How much cycles one 'wait' loop takes
        // This is approximate value. This only needed to guard against too long waits.
        WAIT_LOOP_CYCLES                    = 6,

        // Maximum time for a TX slot
        TX_SLOT_MAX_US                      = 120,

        // Time we keep line sinked to transmit 0
        TX_BIT_US                           = 45,

        // Commands
        ROM_SEARCH                          = 0xF0,
        ROM_ALARM_SEARCH                    = 0xEC,
        ROM_READ                            = 0x33,
        ROM_MATCH                           = 0x55,
        ROM_SKIP                            = 0xCC,
    };

    // State
    uint8_t first_signal_allowed_iterations;

    // - - - - - - - - - - - - - Public functions - - - - - - - - - - - - - - - - - -
    public:

    FORCE_INLINE void set_alarm_on () {
        helpers.set_alarm_on ();
    }

    FORCE_INLINE void set_alarm_off () {
        helpers.set_alarm_off ();
    }

    /**
     * Get soft timer instance used for onewire.
     */
    FORCE_INLINE STIMER *get_stimer () {
        return &stimer;
    }

    /**
     * Set DQ line low (sink).
     */
    FORCE_INLINE void sink () {
        dq.set_ddr (1);
    }

    /**
     * Release DQ line (set 1).
     */
    FORCE_INLINE void release () {
        dq.set_ddr (0);
    }

    /**
     * Initialize onewire (DQ line).
     */
    FORCE_INLINE void init () {
        dq.set_port (0);
        release ();
    }

    /**
     * Turn error flag on.
     */
    FORCE_INLINE void set_error_on () {
        helpers.set_error_on ();
    }

    /**
     * Turn error flag off.
     */
    FORCE_INLINE void set_error_off () {
        helpers.set_error_off ();
    }

    /**
     * Return true if error occured.
     */
    FORCE_INLINE uint8_t is_error () {
        return helpers.is_error ();
    }

    /**
     * Wait for the given number of loops for the 1 to appear on DQ.
     * Turn error flag on if 1 has not been encountered.
     */
    __attribute__ ((noinline))
    void wait_for1_for_loops (uint16_t loops) {
        // XXX: One loop must be WAIT_LOOP_CYCLES long

        while (loops != 0) {
            if (dq.is_pin ()) {
                return;
            }
            
            loops--;
        }

        set_error_on ();
    }

    /**
     * Wait for the given number of microseconds for 1 to appear on DQ.
     * Sets error on if 1 has not beed detected.
     */
    FORCE_INLINE void wait_for1_for_us (uint16_t us) {
        wait_for1_for_loops (
                (uint64_t)akat_cpu_freq_hz () * (uint64_t)us / (uint64_t)1000000L / (uint64_t)WAIT_LOOP_CYCLES);
    }

    /**
     * Wait for the given number of loops for the 0 to appear on DQ.
     * Sets error on if 0 has not beed detected.
     */
    __attribute__ ((noinline))
    void wait_for0_for_loops (uint16_t loops) {
        // XXX: One loop must be WAIT_LOOP_CYCLES long

        while (loops != 0) {
            if (!dq.is_pin ()) {
                return;
            }
            
            loops--;
        }

        set_error_on ();
    }

    /**
     * Wait for the given number of microseconds for 0 to appear on DQ.
     * Sets error on if 0 has not beed detected.
     */
    FORCE_INLINE void wait_for0_for_us (uint16_t us) {
        wait_for0_for_loops (
                (uint64_t)akat_cpu_freq_hz () * (uint64_t)us  / (uint64_t)1000000L / (uint64_t)WAIT_LOOP_CYCLES);
    }

    /**
     * Read a bit from line.
     * Returns bit (1 or 0). Sets error flag on if error occured.
     */
    uint8_t read_bit () {
        if (helpers.is_slot_started ()) {
            helpers.set_slot_started_off ();
        } else {
            wait_for0_for_us (RX_SLOT_SIGNAL_WAITING_US);
            if (is_error ()) {
                return 0;
            }
        }

        // Skip "header" of the slot. The pulse that is sended by MASTER before each pulse
        akat_delay_us (RX_SLOT_SAMPLING_OFFSET_US);

        // Sample data value
        if (dq.is_pin ()) {
            return 1;
        } else {
            // Line has been sinked. This means that 0 was transmitted.
            // At the end of the slot, line must be released. Wait for it.
            const uint16_t delay = RX_SLOT_MAX_US - RX_SLOT_SAMPLING_OFFSET_US + 2 * AVR_DELAYS_ERROR_US;

            wait_for1_for_us (delay);
            // If line is not released. Something is wrong. Error flag is already on in this case..
            return 0;
        }
    }

    /**
     * Read a byte from line.
     * Returns byte. Sets error flag on in case of error.
     */
    uint8_t read_8bit () {
        uint8_t rc = 0;

        for (uint8_t i = 8; i > 0; i--) {
            uint8_t bit = read_bit ();
            if (is_error()) {
                break;
            }
         
            rc = rc >> 1;
            if (bit) {
                rc |= (uint8_t) 0x80;
            }
        }

        return rc;
    }

    /**
     * Write a bit to line.
     * Sets error on in case of error.
     */
    void write_bit (uint8_t bit) {
        wait_for0_for_us (RX_SLOT_SIGNAL_WAITING_US);
        if (is_error()) {
            return;
        }

        if (!bit) {
            sink ();
        }

        // Skip "header" of the slot. The pulse that is sended by MASTER before each pulse
        akat_delay_us (TX_BIT_US);

        release ();

        // Waiting for 1
        // At the end of the slot, line must be released. Wait for it.
        const uint16_t delay = TX_SLOT_MAX_US - TX_BIT_US + 2 * AVR_DELAYS_ERROR_US;

        wait_for1_for_us (delay);
        // If line is not released. Something is wrong. Return error is already set in this case.
    }

    /**
     * Write 8 bit
     */
    void write_8bit (uint8_t byte) {
        for (uint8_t mi = 8; mi > 0; mi--) {
            uint8_t bit = byte & AKAT_ONE;
           
            // Write bit, return if error
            write_bit (bit);
            if (is_error ()) {
                break;
            }

            byte = byte >> 1;
        }
    }

     /**
     * Write 8 bit and update crc8.
     */
    uint8_t write_8bit_crc8 (uint8_t byte, uint8_t crc) {
        write_8bit (byte);
        if (is_error()) {
            return crc;
        }

        return akat_onewire_crc8 (byte, crc);
    } 

    /**
     * Process search request.
     */
    void handle_search () {
        uint8_t *id_byte = id;

        for (uint8_t i = 8; i > 0; i--) {
            uint8_t byte = eeprom_read_byte (id_byte++);

            for (uint8_t mi = 8; mi > 0; mi--) {
                uint8_t bit = byte & AKAT_ONE;
               
                // Write bit, return if error
                write_bit (bit);
                if (is_error ()) {
                    return;
                }

                // Write complement, return if error
                write_bit (!bit);
                if (is_error ()) {
                    return;
                }

                // Read selection bit, return if error or we are rejected
                uint8_t rbit = read_bit ();
                if (is_error ()) {
                    return;
                }             

                if (bit != rbit) {
                    return;
                }

                byte = byte >> 1;
            }
        }
    }

    /**
     * Process read request.
     */
    void handle_read () {
        uint8_t *id_byte = id;

        for (uint8_t i = 8; i > 0; i--) {
            uint8_t byte = eeprom_read_byte (id_byte++);

            write_8bit (byte);
            if (is_error ()) {
                break;
            }
        }
    }

    /**
     * Responde to the search request if alarm flag is on.
     */
    void handle_alarm_search () {
        if (helpers.is_alarm()) {
            handle_search ();
        }
    }

    /**
     * Match id from master and execute function if matches.
     */
    void handle_match () {
         uint8_t *id_byte = id;

         for (uint8_t i = 8; i > 0; i--) {
            uint8_t rbyte = read_8bit ();
            if (is_error ()) {
                return;
            }

            if (rbyte != eeprom_read_byte (id_byte++)) {
                return;
            }
        }

        handle_function ();
    }

    // - - - - - - - - - - -  - - - - - - - - --  - - - - - -- - - - - -
    // Main method for onewire. This method is invoked when a presence
    // pulse has been issued and slot signal has been detected.
    // This function is called with interrupts disabled
    // and must not enable interrupts.
    FORCE_INLINE void read_and_handle_command () {
        helpers.set_slot_started_on ();

        uint8_t byte = read_8bit ();
        if (is_error()) {
            return;
        }

        switch (byte) {
            case ROM_ALARM_SEARCH:
                handle_alarm_search ();
                break;

            case ROM_SEARCH:
                handle_search ();
                break;

            case ROM_READ:
                handle_read ();
                break;

            case ROM_SKIP:
                handle_function ();
                break;

            case ROM_MATCH:
                handle_match ();
                break;
        }
    }

    // - - --  - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Soft timer handler. This is supposed to be executed with IRQ disabled
    FORCE_INLINE void __stimer_body () {
        switch (helpers.get_stage()) {
            case AKAT_ONEWIRE_WAITING_RESET_MINIMUM:
                // Minimum time needed to detect RESET pulse has passed.
                // Now we have to ensure that this is not short circuite of something.
                stimer.set ((RX_RESET_MAX_US - RX_RESET_MIN_US) / timer_us);
                helpers.set_stage (AKAT_ONEWIRE_WAITING_RESET_MAXIMUM);
                break;

            case AKAT_ONEWIRE_PRESENCE_PULSE_ON:
                // It is enough for presence pulse. Lets release DQ line and start real conversation.
                release ();
                
                // We need to wait until line changes to 1
                wait_for1_for_us (TX_PRESENCE_MAX_US);
                if (is_error ()) {
                    // Short-circuit?
                    helpers.set_stage (AKAT_ONEWIRE_IDLE);
                    set_error_off ();
                } else {
                    // Line was release by all devices
                    helpers.set_ignore_one_change_on ();
                    first_signal_allowed_iterations = RX_FIRST_SIGNAL_MAX_100US;
                    stimer.set (100 / timer_us);
                    helpers.set_stage (AKAT_ONEWIRE_WAITING_FIRST_SIGNAL);
                }

                break;

            case AKAT_ONEWIRE_WAITING_FIRST_SIGNAL:
                // No slot-starting signal has been detected withing meaningful time

                if (first_signal_allowed_iterations-- == 0) {
                    // We can wait no longer
                    helpers.set_stage (AKAT_ONEWIRE_IDLE);
                } else {
                    // Going to try again
                    stimer.set (100 / timer_us);
                }

                break;

            case AKAT_ONEWIRE_IDLE:
            case AKAT_ONEWIRE_WAITING_RESET_MAXIMUM:
                break;
        }
    }

    // - - - - - - - - - - - - - - - -  - - - - - -- - - - - - - - - - - - -- 
    // Handle DQ pin change. Must be called from pin change interrupt (IRQ disabled)
    FORCE_INLINE void handle_dq_change () {
        // This must be the first thing we do in order to get right state
        uint8_t dq_state = dq.is_pin ();

        // Act
        switch (helpers.get_stage()) {
            case AKAT_ONEWIRE_IDLE:
                if (!dq_state) {
                    // This either beginning of PRESENT/RESET impulse or just
                    // a start of some data impulse. 
                    // If DQ is not going to be set to 1 within the following 480us
                    // this this is RESET.
                    stimer.set (RX_RESET_MIN_US / timer_us);
                    helpers.set_stage (AKAT_ONEWIRE_WAITING_RESET_MINIMUM);
                }
                break;

            case AKAT_ONEWIRE_WAITING_RESET_MINIMUM:
                // DQ is set to 1 too soon. It is not RESET.
                helpers.set_stage (AKAT_ONEWIRE_IDLE);
                stimer.cancel ();
                break;

            case AKAT_ONEWIRE_WAITING_RESET_MAXIMUM:
                if (dq_state) {
                    // This was likelly a RESET pulse

                    // Make a pause before sending PRESENCE pulse. This is short pause,
                    // so we must not use soft timer.
                    akat_delay_us (DELAY_BEFORE_PRESENCE_US);               

                    // Now start PRESENCE pulse
                    sink ();               
                    stimer.set (TX_PRESENCE_US / timer_us);
                    helpers.set_stage (AKAT_ONEWIRE_PRESENCE_PULSE_ON);
                } else {
                    // Pin changed from 0 to 1, while we expected change from 0 to 1!
                    // We probably have missed something. This is definetelly not RESET.
                    stimer.cancel ();
                    helpers.set_stage (AKAT_ONEWIRE_IDLE);
                }
                break;

            case AKAT_ONEWIRE_WAITING_FIRST_SIGNAL:
                // A slot signal since has been detected
                if (helpers.is_ignore_one_change()) {
                    helpers.set_ignore_one_change_off ();
                    break;
                }

                stimer.cancel ();
                helpers.set_stage (AKAT_ONEWIRE_IDLE);

                read_and_handle_command ();
                set_error_off ();

                break;

            case AKAT_ONEWIRE_PRESENCE_PULSE_ON:
                // Ignore DQ while we switching it ourselves.
                break;
        }
    }
};

#define AKAT_DEFINE_ONEWIRE(name, id, dq, timer_us, f, reg1, reg2, reg3)     \
    void __onewire__stimer_body__##name ();                                  \
                                                                             \
    AKAT_STIMER_8BIT (__onewire_stimer__##name, reg1) {                      \
        __onewire__stimer_body__##name  ();                                  \
    }                                                                        \
                                                                             \
    register akat_onewire_stage_t g_onewire_stage_##name##__ asm(reg2);      \
    register akat_onewire_flags_t g_onewire_flags_##name##__ asm(reg3);      \
                                                                             \
    struct __onewire_helpers_##name##_t__ {                                  \
        /* stage */                                                          \
        FORCE_INLINE akat_onewire_stage_t get_stage () {                     \
            return g_onewire_stage_##name##__;                               \
        }                                                                    \
                                                                             \
        FORCE_INLINE void set_stage (akat_onewire_stage_t __s)               \
        {                                                                    \
            g_onewire_stage_##name##__ = __s;                                \
        }                                                                    \
                                                                             \
        /* alarm */                                                          \
        FORCE_INLINE void set_alarm_on () {                                  \
            g_onewire_flags_##name##__.alarm = 1;                            \
        }                                                                    \
                                                                             \
        FORCE_INLINE void set_alarm_off () {                                 \
            g_onewire_flags_##name##__.alarm = 0;                            \
        }                                                                    \
                                                                             \
        FORCE_INLINE uint8_t is_alarm () {                                   \
            return g_onewire_flags_##name##__.alarm;                         \
        }                                                                    \
                                                                             \
        /* ignore_one_change */                                              \
        FORCE_INLINE void set_ignore_one_change_on () {                      \
            g_onewire_flags_##name##__.ignore_one_change = 1;                \
        }                                                                    \
                                                                             \
        FORCE_INLINE void set_ignore_one_change_off () {                     \
            g_onewire_flags_##name##__.ignore_one_change = 0;                \
        }                                                                    \
                                                                             \
        FORCE_INLINE uint8_t is_ignore_one_change () {                       \
            return g_onewire_flags_##name##__.ignore_one_change;             \
        }                                                                    \
                                                                             \
        /* error */                                                          \
        FORCE_INLINE void set_error_on () {                                  \
            g_onewire_flags_##name##__.error = 1;                            \
        }                                                                    \
                                                                             \
        FORCE_INLINE void set_error_off () {                                 \
            g_onewire_flags_##name##__.error = 0;                            \
        }                                                                    \
                                                                             \
        FORCE_INLINE uint8_t is_error () {                                   \
            return g_onewire_flags_##name##__.error;                         \
        }                                                                    \
                                                                             \
        /* slot_started */                                                   \
        FORCE_INLINE void set_slot_started_on () {                           \
            g_onewire_flags_##name##__.slot_started = 1;                     \
        }                                                                    \
                                                                             \
        FORCE_INLINE void set_slot_started_off () {                          \
            g_onewire_flags_##name##__.slot_started = 0;                     \
        }                                                                    \
                                                                             \
        FORCE_INLINE uint8_t is_slot_started () {                            \
            return g_onewire_flags_##name##__.slot_started;                  \
        }                                                                    \
    } __onewire_helpers_##name##__;                                          \
                                                                             \
    struct akat_onewire_t<typeof(dq),                                        \
                          dq,                                                \
                          typeof(__onewire_stimer__##name),                  \
                          __onewire_stimer__##name,                          \
                          timer_us,                                          \
                          __onewire_helpers_##name##_t__,                    \
                          __onewire_helpers_##name##__,                      \
                          id,                                                \
                          f>  name;                                          \
                                                                             \
    void __onewire__stimer_body__##name () {                                 \
        name.__stimer_body ();                                               \
    }

/**
 * CRC calculation algorithm.
 */
template<uint8_t crc_accum, uint8_t byte>
struct akat_onewire_crc {
    enum {
        __xored__ = crc_accum ^ byte,
        value =
            __xored__ == 0   ? 0 :
            __xored__ == 1   ? 94 :
            __xored__ == 2   ? 188 :
            __xored__ == 3   ? 226 :
            __xored__ == 4   ? 97 :
            __xored__ == 5   ? 63 :
            __xored__ == 6   ? 221 :
            __xored__ == 7   ? 131 :
            __xored__ == 8   ? 194 :
            __xored__ == 9   ? 156 :
            __xored__ == 10  ? 126 :
            __xored__ == 11  ? 32 :
            __xored__ == 12  ? 163 :
            __xored__ == 13  ? 253 :
            __xored__ == 14  ? 31 :
            __xored__ == 15  ? 65 :
            __xored__ == 16  ? 157 :
            __xored__ == 17  ? 195 :
            __xored__ == 18  ? 33 :
            __xored__ == 19  ? 127 :
            __xored__ == 20  ? 252 :
            __xored__ == 21  ? 162 :
            __xored__ == 22  ? 64 :
            __xored__ == 23  ? 30 :
            __xored__ == 24  ? 95 :
            __xored__ == 25  ? 1 :
            __xored__ == 26  ? 227 :
            __xored__ == 27  ? 189 :
            __xored__ == 28  ? 62 :
            __xored__ == 29  ? 96 :
            __xored__ == 30  ? 130 :
            __xored__ == 31  ? 220 :
            __xored__ == 32  ? 35 :
            __xored__ == 33  ? 125 :
            __xored__ == 34  ? 159 :
            __xored__ == 35  ? 193 :
            __xored__ == 36  ? 66 :
            __xored__ == 37  ? 28 :
            __xored__ == 38  ? 254 :
            __xored__ == 39  ? 160 :
            __xored__ == 40  ? 225 :
            __xored__ == 41  ? 191 :
            __xored__ == 42  ? 93 :
            __xored__ == 43  ? 3 :
            __xored__ == 44  ? 128 :
            __xored__ == 45  ? 222 :
            __xored__ == 46  ? 60 :
            __xored__ == 47  ? 98 :
            __xored__ == 48  ? 190 :
            __xored__ == 49  ? 224 :
            __xored__ == 50  ? 2 :
            __xored__ == 51  ? 92 :
            __xored__ == 52  ? 223 :
            __xored__ == 53  ? 129 :
            __xored__ == 54  ? 99 :
            __xored__ == 55  ? 61 :
            __xored__ == 56  ? 124 :
            __xored__ == 57  ? 34 :
            __xored__ == 58  ? 192 :
            __xored__ == 59  ? 158 :
            __xored__ == 60  ? 29 :
            __xored__ == 61  ? 67 :
            __xored__ == 62  ? 161 :
            __xored__ == 63  ? 255 :
            __xored__ == 64  ? 70 :
            __xored__ == 65  ? 24 :
            __xored__ == 66  ? 250 :
            __xored__ == 67  ? 164 :
            __xored__ == 68  ? 39 :
            __xored__ == 69  ? 121 :
            __xored__ == 70  ? 155 :
            __xored__ == 71  ? 197 :
            __xored__ == 72  ? 132 :
            __xored__ == 73  ? 218 :
            __xored__ == 74  ? 56 :
            __xored__ == 75  ? 102 :
            __xored__ == 76  ? 229 :
            __xored__ == 77  ? 187 :
            __xored__ == 78  ? 89 :
            __xored__ == 79  ? 7 :
            __xored__ == 80  ? 219 :
            __xored__ == 81  ? 133 :
            __xored__ == 82  ? 103 :
            __xored__ == 83  ? 57 :
            __xored__ == 84  ? 186 :
            __xored__ == 85  ? 228 :
            __xored__ == 86  ? 6 :
            __xored__ == 87  ? 88 :
            __xored__ == 88  ? 25 :
            __xored__ == 89  ? 71 :
            __xored__ == 90  ? 165 :
            __xored__ == 91  ? 251 :
            __xored__ == 92  ? 120 :
            __xored__ == 93  ? 38 :
            __xored__ == 94  ? 196 :
            __xored__ == 95  ? 154 :
            __xored__ == 96  ? 101 :
            __xored__ == 97  ? 59 :
            __xored__ == 98  ? 217 :
            __xored__ == 99  ? 135 :
            __xored__ == 100 ? 4 :
            __xored__ == 101 ? 90 :
            __xored__ == 102 ? 184 :
            __xored__ == 103 ? 230 :
            __xored__ == 104 ? 167 :
            __xored__ == 105 ? 249 :
            __xored__ == 106 ? 27 :
            __xored__ == 107 ? 69 :
            __xored__ == 108 ? 198 :
            __xored__ == 109 ? 152 :
            __xored__ == 110 ? 122 :
            __xored__ == 111 ? 36 :
            __xored__ == 112 ? 248 :
            __xored__ == 113 ? 166 :
            __xored__ == 114 ? 68 :
            __xored__ == 115 ? 26 :
            __xored__ == 116 ? 153 :
            __xored__ == 117 ? 199 :
            __xored__ == 118 ? 37 :
            __xored__ == 119 ? 123 :
            __xored__ == 120 ? 58 :
            __xored__ == 121 ? 100 :
            __xored__ == 122 ? 134 :
            __xored__ == 123 ? 216 :
            __xored__ == 124 ? 91 :
            __xored__ == 125 ? 5 :
            __xored__ == 126 ? 231 :
            __xored__ == 127 ? 185 :
            __xored__ == 128 ? 140 :
            __xored__ == 129 ? 210 :
            __xored__ == 130 ? 48 :
            __xored__ == 131 ? 110 :
            __xored__ == 132 ? 237 :
            __xored__ == 133 ? 179 :
            __xored__ == 134 ? 81 :
            __xored__ == 135 ? 15 :
            __xored__ == 136 ? 78 :
            __xored__ == 137 ? 16 :
            __xored__ == 138 ? 242 :
            __xored__ == 139 ? 172 :
            __xored__ == 140 ? 47 :
            __xored__ == 141 ? 113 :
            __xored__ == 142 ? 147 :
            __xored__ == 143 ? 205 :
            __xored__ == 144 ? 17 :
            __xored__ == 145 ? 79 :
            __xored__ == 146 ? 173 :
            __xored__ == 147 ? 243 :
            __xored__ == 148 ? 112 :
            __xored__ == 149 ? 46 :
            __xored__ == 150 ? 204 :
            __xored__ == 151 ? 146 :
            __xored__ == 152 ? 211 :
            __xored__ == 153 ? 141 :
            __xored__ == 154 ? 111 :
            __xored__ == 155 ? 49 :
            __xored__ == 156 ? 178 :
            __xored__ == 157 ? 236 :
            __xored__ == 158 ? 14 :
            __xored__ == 159 ? 80 :
            __xored__ == 160 ? 175 :
            __xored__ == 161 ? 241 :
            __xored__ == 162 ? 19 :
            __xored__ == 163 ? 77 :
            __xored__ == 164 ? 206 :
            __xored__ == 165 ? 144 :
            __xored__ == 166 ? 114 :
            __xored__ == 167 ? 44 :
            __xored__ == 168 ? 109 :
            __xored__ == 169 ? 51 :
            __xored__ == 170 ? 209 :
            __xored__ == 171 ? 143 :
            __xored__ == 172 ? 12 :
            __xored__ == 173 ? 82 :
            __xored__ == 174 ? 176 :
            __xored__ == 175 ? 238 :
            __xored__ == 176 ? 50 :
            __xored__ == 177 ? 108 :
            __xored__ == 178 ? 142 :
            __xored__ == 179 ? 208 :
            __xored__ == 180 ? 83 :
            __xored__ == 181 ? 13 :
            __xored__ == 182 ? 239 :
            __xored__ == 183 ? 177 :
            __xored__ == 184 ? 240 :
            __xored__ == 185 ? 174 :
            __xored__ == 186 ? 76 :
            __xored__ == 187 ? 18 :
            __xored__ == 188 ? 145 :
            __xored__ == 189 ? 207 :
            __xored__ == 190 ? 45 :
            __xored__ == 191 ? 115 :
            __xored__ == 192 ? 202 :
            __xored__ == 193 ? 148 :
            __xored__ == 194 ? 118 :
            __xored__ == 195 ? 40 :
            __xored__ == 196 ? 171 :
            __xored__ == 197 ? 245 :
            __xored__ == 198 ? 23 :
            __xored__ == 199 ? 73 :
            __xored__ == 200 ? 8 :
            __xored__ == 201 ? 86 :
            __xored__ == 202 ? 180 :
            __xored__ == 203 ? 234 :
            __xored__ == 204 ? 105 :
            __xored__ == 205 ? 55 :
            __xored__ == 206 ? 213 :
            __xored__ == 207 ? 139 :
            __xored__ == 208 ? 87 :
            __xored__ == 209 ? 9 :
            __xored__ == 210 ? 235 :
            __xored__ == 211 ? 181 :
            __xored__ == 212 ? 54 :
            __xored__ == 213 ? 104 :
            __xored__ == 214 ? 138 :
            __xored__ == 215 ? 212 :
            __xored__ == 216 ? 149 :
            __xored__ == 217 ? 203 :
            __xored__ == 218 ? 41 :
            __xored__ == 219 ? 119 :
            __xored__ == 220 ? 244 :
            __xored__ == 221 ? 170 :
            __xored__ == 222 ? 72 :
            __xored__ == 223 ? 22 :
            __xored__ == 224 ? 233 :
            __xored__ == 225 ? 183 :
            __xored__ == 226 ? 85 :
            __xored__ == 227 ? 11 :
            __xored__ == 228 ? 136 :
            __xored__ == 229 ? 214 :
            __xored__ == 230 ? 52 :
            __xored__ == 231 ? 106 :
            __xored__ == 232 ? 43 :
            __xored__ == 233 ? 117 :
            __xored__ == 234 ? 151 :
            __xored__ == 235 ? 201 :
            __xored__ == 236 ? 74 :
            __xored__ == 237 ? 20 :
            __xored__ == 238 ? 246 :
            __xored__ == 239 ? 168 :
            __xored__ == 240 ? 116 :
            __xored__ == 241 ? 42 :
            __xored__ == 242 ? 200 :
            __xored__ == 243 ? 150 :
            __xored__ == 244 ? 21 :
            __xored__ == 245 ? 75 :
            __xored__ == 246 ? 169 :
            __xored__ == 247 ? 247 :
            __xored__ == 248 ? 182 :
            __xored__ == 249 ? 232 :
            __xored__ == 250 ? 10 :
            __xored__ == 251 ? 84 :
            __xored__ == 252 ? 215 :
            __xored__ == 253 ? 137 :
            __xored__ == 254 ? 107 :
            __xored__ == 255 ? 53 : 0
    };
};


#endif
