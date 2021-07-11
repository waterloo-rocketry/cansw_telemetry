#include "interrupts.h"
#include "radio.h"
#include <xc.h>
#include <stdint.h>

//void __interrupt() isr(void)
//{
//    // Check if it was the SPI interrupt
//    if ( PIE1bits.SSP1IE == 1 && PIR1bits.SSP1IF == 1) {
//        radio_interrupt_handler();
//        PIR1bits.SSP1IF = 0;
//    } else {
//        //unhandled interrupt. No idea what to do here, so just infinite loop
//        while (1);
//    }
//}