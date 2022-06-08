#include "platform.h"
#include <xc.h>

// whether the leds turn on when the pin is set to high or low
#define LED_ON 0

void gpio_init(void) {
    // set as outputs
    TRISC2 = 0; // LED
    TRISC3 = 0; // LED
    TRISC4 = 0; // LED
    // turn LEDs off
    LATC2 = !LED_ON;
    LATC3 = !LED_ON;
    LATC4 = !LED_ON;
    
    TRISA4 = 0; // 12V EN
    TRISA5 = 0; //  5V EN
    TRISA3 = 0; // GND EN
    // initiallly power off bus
    LATA4 = 0;
    LATA5 = 0;
    LATA3 = 0;
}

void RED_LED_SET(bool value) {
    LATC3 = !value ^ LED_ON;
}
void BLUE_LED_SET(bool value) {
    LATC4 = !value ^ LED_ON;
}
void WHITE_LED_SET(bool value) {
    LATC2 = !value ^ LED_ON;
}

void SET_BUS_POWER(bool value) {
    LATA4 = value;
    LATA5 = value;
    LATA3 = value;
}
