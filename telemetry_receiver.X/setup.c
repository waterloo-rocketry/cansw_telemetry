#include <stdbool.h>

#include "setup.h"


void init_leds(void){
    // White LED Setup
    TRISA4 = 0; // Set white LED as output
    LATA4 = 1;
    
    // Red LED Setup
    TRISA5 = 0; // Set red LED as output
    LATA5 = 1;
    
    // Blue LED Setup
    TRISC5 = 0; // Set blue LED as output
    LATC5 = 1;
    
    /* // For Keto
    // White LED Setup
    TRISB4 = 0; // Set white LED as output
    ANSELB4 = 0; // Enable digital input buffer
    
    // Red LED Setup
    TRISB3 = 0; // Set red LED as output
    ANSELB3 = 0; // Enable digital input buffer
    
    // Blue LED Setup
    TRISB2 = 0; // Set blue LED as output
    ANSELB2 = 0; // Enable digital input buffer

    */
}

/* Blue LED set to heartbeat. */
void LED_HEARTBEAT(void)
{
    static bool led_on = false;
    if (led_on) {
        BLUE_LED_ON();
        led_on = false;
    } else {
        BLUE_LED_OFF();
        led_on = true;
    }
}