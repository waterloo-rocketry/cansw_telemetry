#include "spi.h"
#include <stdint.h>
#include <stdio.h>

void init_spi(void){
    // SSP Status Register
    SSPSTAT = 0b10000000; // select master mode for data input, clock edge select, and buffer full status
    // SSP Config Register 1
    SSPCON1 =0b00110010; // no buffer overflow, enables serial port, set idle clock state, set master mode
    // SSP Config Register 2
    SSPCON2 = 0b00000000; // set start/stop condition to idle
    
    ANSELC &= ~(0x15); // Need to clear relevant ANSELC pins to 0 to use digitally
    
    // SDO pin to output
    TRISC2 = 0;
    // SDI pin to input
    TRISC1 = 1;
    // SCK pin to output
    TRISC0 = 0;
    // SS pin to output
    TRISC3 = 0;       
}


void spi_write_byte(uint8_t byte){
    uint8_t temp = SSPBUF;
    PIR1bits.SSP1IF = 0;
    SSPCONbits.WCOL = 0;
    SSPBUF = byte;
    while(!PIR1bits.SSP1IF) {};
}

void spi_write_buffer(uint8_t *data, uint8_t length){
    while (length--) {
        spi_write_byte(*data);
        data++;
    }
}

uint8_t spi_read_byte(void) {
    // Need to read the SSPBUF first
    uint8_t temp = SSPBUF;
    PIR1bits.SSP1IF = 0;
    //clock 0 onto the pins
    SSPBUF = 0;
    while(!PIR1bits.SSP1IF) {}
    return SSPBUF;
}
