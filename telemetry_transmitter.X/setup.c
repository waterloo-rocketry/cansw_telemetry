#include "setup.h"


void init_leds(void){
    // White LED Setup
    TRISC2 = 0; // Set white LED as output
    ANSELC2 = 0; // Enable digitali nput buffer
    LATC2 = 1; // Turn white LED off
    
    // Red LED Setup
    TRISC3 = 0; // Set red LED as output
    ANSELC3 = 0; // Enable digital input buffer
    LATC3 = 1; // Turn red LED off
    
    // Blue LED Setup
    TRISC4 = 0; // Set blue LED as output
    ANSELC4 = 0; // Enable digital input buffer
    LATC4 = 1; // Turn blue LED off
    
    /*
    // White LED Setup
    TRISB4 = 0; // Set white LED as output
    ANSELB4 = 0; // Enable digital input buffer
    LATB4 = 1; // Turn white LED off
    
    // Red LED Setup
    TRISB3 = 0; // Set red LED as output
    ANSELB3 = 0; // Enable digital input buffer
    LATB3 = 1; // Turn red LED off
    
    // Blue LED Setup
    TRISB2 = 0; // Set blue LED as output
    ANSELB2 = 0; // Enable digital input buffer
    LATB2 = 1; // Turn blue LED off*/
    
}

void init_osc(void){
    //Select external oscillator with PLL of 1:1
    OSCCON1 = 0b01110000;
    //wait until the clock switch has happened
    while (OSCCON3bits.ORDY == 0)  {}
    //if the currently active clock (CON2) isn't the selected clock (CON1)
    if (OSCCON2 != 0b01110000) {
        //infinite loop, something is broken, what even is an assert()?
        while (1) {}
    }
}

void init_interrupts(void){
    // Enable global interrupts
    INTCON0bits.GIE = 1;
    
    // Enable SPI interrupts
    
}
