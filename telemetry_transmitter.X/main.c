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

    int msg_counter = 0;
    char transmit_buffer[25];
//    char receive_buffer[25];

    // Main Loop
    while (1)
    {
        // LED Blinky
        WHITE_LED_ON();
        __delay_ms(1000);
        WHITE_LED_OFF();
        RED_LED_ON();
        __delay_ms(1000);
        RED_LED_OFF();
        BLUE_LED_ON();
        __delay_ms(1000);
        BLUE_LED_OFF();
        
        
//        sprintf(transmit_buffer, "LTR 1: %i\n", msg_counter);
        
        // Check if data has been received
//        if (radio_receive_data(receive_buffer, sizeof (receive_buffer))) {
//            // Send a reply
//            sprintf(transmit_buffer, "ID:0x%X/RX:%d/TX:%d\n", CURR_RADIO_NODE_ID, _num_rx_good_msgs, _num_tx_good_msgs);
//            status = radio_transmit_data(transmit_buffer, sizeof (transmit_buffer));
//        }
        
        __delay_ms(1000);
//        msg_counter++;
    }
    return 0;
}