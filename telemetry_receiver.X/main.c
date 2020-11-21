#include <stdio.h>
#include <stdlib.h>

#include "mcc_generated_files/mcc.h"
#include "spi.h"
#include "setup.h"
#include <xc.h>

#define _XTAL_FREQ 48000000

int main(void)
{
    // Initialization functions
    SYSTEM_Initialize();
    init_leds();
    
    // Enable the Global Interrupts
    INTERRUPT_GlobalInterruptEnable();

    // Enable the Peripheral Interrupts
    INTERRUPT_PeripheralInterruptEnable();
    
    // Enable MSSP (SPI) Interrupt 
    PIE1bits.SSP1IE = 1;

    // Main Loop
    while (1)
    {        
        
        /* // LED Blinking
        WHITE_LED_ON();
        __delay_ms(1000);
        WHITE_LED_OFF();
        RED_LED_ON();
        __delay_ms(1000);
        RED_LED_OFF();
        BLUE_LED_ON();
        __delay_ms(1000);
        BLUE_LED_OFF();*/
    }
    return 0;
}