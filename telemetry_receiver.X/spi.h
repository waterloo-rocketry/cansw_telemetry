#ifndef SPI_H
#define	SPI_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <xc.h>
#include <stdint.h>
    
/*
 * Initialize the SPI module. Set up TX and RX buffers, and set up the module.
 */
void spi_init(void);

/*
 * Send a byte over SPI.
 */
void spi_write_byte(uint8_t data);

/* Sends multiple bytes of data (from buffer). 
 *
 * data: pointer to an array of bytes to send
 * length: the number of bytes that should be sent from that array
 */
void spi_write_buffer(uint8_t *buffer, size_t length);

/*
 * Receive a byte over SPI.
 */
uint8_t spi_read_byte(void);

#endif	/* SPI_H */

