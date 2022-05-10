#include "led_manager.h"
#include "setup.h"
#include "bus_power.h"
#include "sotscon.h"
#include "timer.h"
#include "radio_handler.h"
#include <stdint.h>
#include <stdbool.h>

static uint32_t time_last_blink_started = 0;
static bool vent_valve_desired_open, inj_valve_desired_open, bus_powered;

void init_led_manager(void)
{
    WHITE_LED_OFF();
    RED_LED_OFF();
    BLUE_LED_OFF();

    time_last_blink_started = millis();
    if (radio_get_expected_vent_valve_state() == VALVE_OPEN)
    {
        vent_valve_desired_open = true;
    }
    if (radio_get_expected_inj_valve_state() == VALVE_OPEN)
    {
        inj_valve_desired_open = true;
    }
    bus_powered = is_bus_powered();
}

void led_manager_heartbeat(void)
{
    // everything has a frequency of 0.5 Hz
    if (millis() - time_last_blink_started > 2000)
    {
        time_last_blink_started = millis();

        // only decide these once per cycle to save function calls
        vent_valve_desired_open = (radio_get_expected_vent_valve_state() == VALVE_OPEN);
        inj_valve_desired_open = (radio_get_expected_inj_valve_state() == VALVE_OPEN);
        bus_powered = is_bus_powered();
    }

    // from time 0 to 100 ms:
    // WHITE_LED is on
    // RED_LED is on if we want the vent valve open
    // BLUE_LED is on if we want the injector valve open
    if (millis() - time_last_blink_started < 100)
    {
        WHITE_LED_ON();
        if (vent_valve_desired_open)
        {
            RED_LED_ON();
        }
        if (inj_valve_desired_open)
        {
            BLUE_LED_ON();
        }
    }

    // from time 100 ms to 300ms, everything is off
    else if (millis() - time_last_blink_started < 300)
    {
        WHITE_LED_OFF();
        RED_LED_OFF();
        BLUE_LED_OFF();
    }

    // from time 300 to 400ms:
    // WHITE_LED is on if the bus is powered
    else if (millis() - time_last_blink_started < 400)
    {
        if (bus_powered)
        {
            WHITE_LED_ON();
        }
    }

    // from time 400ms to 2000ms, everything is off
    else
    {
        WHITE_LED_OFF();
        RED_LED_OFF();
        BLUE_LED_OFF();
    }
}
