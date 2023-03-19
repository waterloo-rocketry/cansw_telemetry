#include "platform.h"
#include <xc.h>

// whether the leds turn on when the pin is set to high or low
#define LED_ON 0

void gpio_init(void) {
    // set as outputs
    TRISC5 = 0; // LED
    TRISC6 = 0; // LED
    TRISC7 = 0; // LED
    // turn LEDs off
    LATC5 = !LED_ON;
    LATC6 = !LED_ON;
    LATC7 = !LED_ON;
    
    TRISA4 = 1;
    // initially power off radio
    TRISC4 = 0; // LED
    LATC4 = 0;
}

void RED_LED_SET(bool value) {
    LATC6 = !value ^ LED_ON;
}
void BLUE_LED_SET(bool value) {
    LATC7 = !value ^ LED_ON;
}
void WHITE_LED_SET(bool value) {
    LATC5 = !value ^ LED_ON;
}

void SET_RADIO_POWER(bool value) {
    LATC4 = value;
    RED_LED_SET(value);
}
