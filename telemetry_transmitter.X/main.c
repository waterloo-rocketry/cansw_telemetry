#include <stdio.h>
#include <stdlib.h>

#include "mcc_generated_files/mcc.h"
//#include "spi.h"
#include "setup.h"
#include <xc.h>

#define _XTAL_FREQ 12000000 // 12MHz



int main(void)
{
    // Initialization functions
    SYSTEM_Initialize();
    init_leds();
    //init_spi();

    // Main Loop
    while (1)
    {
        WHITE_LED_ON();
        __delay_ms(5000);
        WHITE_LED_OFF();
        RED_LED_ON();
        __delay_ms(5000);
        RED_LED_OFF();
        BLUE_LED_ON();
        __delay_ms(5000);
        BLUE_LED_OFF();
    }
    return 0;
}