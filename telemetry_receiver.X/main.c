#include <stdio.h>
#include <stdlib.h>

#include <xc.h>
#include "mcc_generated_files/mcc.h"

#include "radio.h"
#include "radio_config.h"
#include "spi.h"
#include "setup.h"
#include "usb_app.h"
#include "timer.h"

// Data buffers
//static char input_string[64];

int main(void) {
    bool status = false;

    // Initialization functions
    SYSTEM_Initialize();
    init_leds();

    // Enable the Global Interrupts
    INTERRUPT_GlobalInterruptEnable();

    // Enable the Peripheral Interrupts
    INTERRUPT_PeripheralInterruptEnable();

    // Enable MSSP (SPI) Interrupt 
    PIE1bits.SSP1IE = 1;
    
    // init our millisecond function
    timer0_init();
    
    // Set baud rate
    CDCSetBaudRate(9600);
//    // Check if primary transceiver
//    if (CURR_RADIO_NODE_ADDRESS == 0x01) {
//        // Send a message
//        sprintf(transmit_buffer, "ID: 0x%X, Sending init msg!\n", CURR_RADIO_NODE_ID);
//        status = radio_transmit_data(transmit_buffer, sizeof (transmit_buffer));
//
//        if (status == false) {
//            strcpy(transmit_buffer, "ERROR: Couldn't send init msg...\n");
//            usb_app_write_string(transmit_buffer, sizeof (transmit_buffer));
//        }
//    }
    
    int msg_counter = 0;
    char transmit_buffer[25];
//    char receive_buffer[25];

    // Main Loop
    while (1) {
//        if (usb_app_available_bytes() != 0) {
//            usb_app_read_bytes(input_string, sizeof (input_string));
//        }

        sprintf(transmit_buffer, "LTR 1: %i\n", msg_counter);
        usb_app_write_string(transmit_buffer, sizeof(transmit_buffer));

//        if (radio_receive_data(receive_buffer, sizeof (receive_buffer))) {
//            // Got message!
//            usb_app_write_string(receive_buffer, sizeof (receive_buffer));
//
//            // Send a reply!
//            sprintf(transmit_buffer, "ID:0x%X/RX:%d/TX:%d\n", CURR_RADIO_NODE_ID, _num_rx_good_msgs, _num_tx_good_msgs);
//            status = radio_transmit_data(transmit_buffer, sizeof (transmit_buffer));
//
//            if (status == false) {
//                strcpy(transmit_buffer, "ERROR sending msg...\n");
//                usb_app_write_string(transmit_buffer, sizeof (transmit_buffer));
//            }
//        }
        
        LED_HEARTBEAT();
        usb_app_heartbeat();
        __delay_ms(1000);
        msg_counter++;
    }
    return 0;
}