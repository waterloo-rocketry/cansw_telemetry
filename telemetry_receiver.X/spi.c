#include "spi.h"
#include <stdint.h>
#include <stdio.h>


void spi_init(void){
    // SSP Status Register
    SSPSTAT = 0b10000000; // select master mode for data input, clock edge select, and buffer full status
    // SSP Config Register 1
    SSPCON1 = 0b00100010; // no buffer overflow, enables serial port, set idle clock state, set master mode
    // SSP Config Register 2
    SSPCON2 = 0b00000000; // set start/stop condition to idle
    
    ANSELC &= ~(0xf); // Need to clear relevant ANSELC pins to 0 to use digitally (pins 0-3)
    
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
    // Get any unused contents of the SPBUF before writing
    uint8_t temp = SSPBUF;
    // Reset the interrupt flag bit
    PIR1bits.SSP1IF = 0;
    // Reset write collision detect bit 
    SSPCONbits.WCOL = 0;
    // Put byte into buffer to be sent
    SSPBUF = byte;
    // Wait until interrupt flag bit is set (i.e. byte is written)
    while(!PIR1bits.SSP1IF) {};
}

void spi_write_buffer(const uint8_t *buffer, size_t len){ 
    uint8_t *data = buffer;
    while (len--) {
        spi_write_byte(*data++);
    }
}

uint8_t spi_read_byte(void) {
    // Get contents of the SSPBUF before reading
    uint8_t temp = SSPBUF;  // TBH I do not remember why this needs to be done
    // Reset the interrupt flag bit
    PIR1bits.SSP1IF = 0;
    // Reset the buffer
    SSPBUF = 0;
    // Wait until the interrupt flag bit is set and we've received all data
    while(!PIR1bits.SSP1IF) {}
    // Return received data
    return SSPBUF;   
}
