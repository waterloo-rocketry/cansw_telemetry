#include "analog.h"
#include "platform.h"
#include "pic18_time.h"
#include <xc.h>

//global variables. Until we have a better way of handling analog
//values that we read (putting them out over CAN or something), just
//store them here
static uint16_t analog_last_batt_voltage = 0;
static uint16_t analog_last_batt_current = 0;
static uint16_t analog_last_bus_current = 0;

uint16_t analog_get_vin_mv(void)
{
    /*
     * the ADC is reading a signal that's divided through a 75K->10K voltage
     * divider. It's comparing that value to a 4.096V reference voltage with a
     * 12bit ADC. Let vpin be the voltage at the pin (post divider) and vbatt be
     * the voltage before the divider
     *
     * vpin = analog_last_batt_voltage * 4.096 / 4096 vbatt = vpin / ( (75k +
     * 10k) / 75k * 10k) vbatt = vpin / (85/750) vbatt =
     * analog_last_batt_voltage / (85/750) vbatt = analog_last_batt_voltage *
     * 8.82352
     *
     * that math is probably good, but experimentally we were seeing a value of
     * 1072 on analog_last_batt value when voltage was 9.15, so we're gonna use
     * a multiplication factor of 8.54 instead
     *
     * TODO, more rigourous measurement of what that multiplication factor
     * should be
     */
    return analog_last_batt_voltage * 8.54;
}

uint16_t analog_get_ibatt_ma(void)
{
    /*
     * the ADC is reading a signal that has been amplified 100x. The sense
     * resistors are 150mOhm. Therefore, 1mA = 150uV * 100 = 15mV.
     *
     * one ADC read is 1mV (12 bit gives 4096 possible numbers, and we're
     * comparing to a voltage of 4.096V. Therefore, to go from
     * analog_last_batt_current to battery current in mA, we have to divide by
     * 15.
     *
     * Anecdotally, I ran this in debug mode and found that we were getting
     * anywhere between 14 and 24 mA, and then I ran it in normal mode and
     * measured the current draw, and got abotu 22mA (jumping up to 27 mA when
     * an LED turned on, as you'd expect). So I'm calling this division thing
     * pretty accurate.
     */
    return analog_last_batt_current / 15;
}

uint16_t analog_get_ibus_ma(void)
{
    /*
     * Same logic as above. Divide by 15.
     */
    return analog_last_bus_current / 15;
}

//the states for the adc state machine thing
static enum {
    NONE,
    READING_BATT_VOLTAGE,
    READING_BATT_CURRENT,
    READING_BUS_CURRENT
} state;


void analog_report_value(uint16_t value, uint8_t channel)
{
    switch (state) {
        case READING_BATT_VOLTAGE:
            //confirm that channel is the battery voltage channel
            if (channel != ANALOG_CH_BATT_VOLTAGE) {
                //this shouldn't be possible, so infinite loop to mark
                //that someday we'll have better error reporting than
                //infinite loops, but i don't know when
                while (1) {}
            }

            //here is where we would report the value over CAN or
            //something, but we don't have any way to do that. So... TODO
            analog_last_batt_voltage = value;

            //transition to the next state. After reading the battery
            //voltage, we switch to reading the battery current, and then
            //to reading the bus current.
            state = READING_BATT_CURRENT;

            //set up the next reading. Set the analog channel and hit go
            ADPCH = ANALOG_CH_BATT_CURRENT;
            ADCON0bits.GO = 1;

            break;
        case READING_BATT_CURRENT:
            //confirm that channel is the battery current channel
            if (channel != ANALOG_CH_BATT_CURRENT) {
                while (1) {}
            }

            analog_last_batt_current = value;

            state = READING_BUS_CURRENT;

            ADPCH = ANALOG_CH_BUS_CURRENT;
            ADCON0bits.GO = 1;

            break;
        case READING_BUS_CURRENT:
            if (channel != ANALOG_CH_BUS_CURRENT) {
                while (1) {}
            }

            analog_last_bus_current = value;

            state = NONE;

            break;
        case NONE:
            //this should never happen. This function should never get
            //called unless we're in a state that isn't NONE.
            while (1) {}
            break;
        default:
            //also an unreachable state
            while (1) {}
    }
}

bool analog_read_all_values()
{
    if (state == NONE) {
        //this is the only time that this function is allowed to write
        //to the state variable
        state = READING_BATT_VOLTAGE;
        //start the analog conversion process.
        // set channel to ANA3, the battery voltage channel
        ADPCH = ANALOG_CH_BATT_VOLTAGE;
        // click go.
        ADCON0bits.GO = 1;
        return true;
    } else {
        return false;
    }
}

void analog_heartbeat(void)
{
    static uint32_t last_read_ms = 0;
    if (millis() - last_read_ms > 100) {
        last_read_ms = millis();
        analog_read_all_values();
    }
}
