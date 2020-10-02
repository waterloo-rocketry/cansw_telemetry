#include "spi.h"
#include <stdint.h>
#include <stdio.h>

void init_spi(void){
    // SPI Module Config Register 0
    SPI1CON0 = 0b10000010; // Enable module, MSB data exchange, and set bus master
    // SPI Module Config Register 1
    SPI1CON1 = 0b00010100; // TODO: write what happens here 
    // SPI Module Config Register 2
    SPI1CON2 = 0b00000011; // set to full duplex, TODO: finish writing what happens here
    // SPI Clock Select 
    //SPI1CLK = 0; // FOSC
    // SPI Baud Rate
    //SPI1BAUD = 0;
    // Transfer Counter
    //SPI1TCNTHbits.BMODE = 1;
    
    // SDO pin to output
    TRISB2 = 0;
    // SDI pin to input
    TRISB1 = 1;
    // SCK pin to output
    TRISB0 = 0;
    // SS pin to output
    TRISC7 = 0;       
}

void spi_write_byte(uint8_t byte){
    SPI1TXB = byte;
}

void spi_write_buffer(uint8_t *data, uint8_t length){
    while (length--) {
        spi_write_byte(*data);
        data++;
    }
}

uint8_t spi_read_byte(void) {
    if (SPI1RXB == 0){
        SPI1STATUSbits.RXRE = 0;
        printf("No data to read.");
        return 0;
    }
    return SPI1RXB;
}
