#include <stdio.h>
#include <stdlib.h>

#include "mcc_generated_files/mcc.h"
#include "mcc_generated_files/device_config.c"
//#include "spi.h"
#include "setup.h"
#include <xc.h>

//#define _XTAL_FREQ 48000000

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
        __delay_ms(1000);
        WHITE_LED_OFF();
        RED_LED_ON();
        __delay_ms(1000);
        RED_LED_OFF();
        BLUE_LED_ON();
        __delay_ms(1000);
        BLUE_LED_OFF();
    }
    return 0;
}