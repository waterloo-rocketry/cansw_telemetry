#ifndef RADIO_CONFIG_H
#define	RADIO_CONFIG_H

#include <stdint.h>
#include <xc.h>

#define _XTAL_FREQ 48000000
/*
 * Node Address of the current node
 */
#define CURR_RADIO_NODE_ADDRESS 0x01 // TODO: Give proper address

/*
 * Node ID of the current node
 */
#define CURR_RADIO_NODE_ID 0x01 // TODO: Give proper IDs

/*
 * Node flags of the current node
 */
#define CURR_RADIO_NODE_FLAGS 0x00 // TODO: Give proper flags

/*
 * Node Addresses of other nodes
 */
const uint8_t DEST_RADIO_NODE_ADDRESSES[] = 
{
    0x02, // ID2
    0x03, // ID3
    0x04  // ID4
};
/*
 * Node Index for node addresses
 */
#define NODE_INDEX 0

/* Max number of bytes the LORa Rx/Tx FIFO can hold */
#define RFM95_FIFO_SIZE 68 // TODO: change to 255 after figuring out memory issues

/* This is the maximum number of bytes that can be carried by the LoRa */
#define RFM95_MAX_PAYLOAD_LEN RFM95_FIFO_SIZE

/* Header length*/
#define RFM95_HEADER_LEN 4

/* 
 * From C++ drivers:
 * This is the maximum message length that can be supported by this driver. 
 * Can be pre-defined to a smaller size (to save SRAM) prior to including this header
 * Here we allow for 1 byte message length, 4 bytes headers, user data and 2 bytes of FCS
 */
#ifndef RFM95_MAX_MESSAGE_LEN
#define RFM95_MAX_MESSAGE_LEN (RFM95_MAX_PAYLOAD_LEN - RFM95_HEADER_LEN)
#endif

/* The crystal oscillator frequency of the module */
#define RFM95_FXOSC 32000000.0

/* The Frequency Synthesizer step = RFM95_FXOSC / 2^^19 */
#define RFM95_FSTEP  (RFM95_FXOSC / 524288)

/* 
 * From C++ drivers:
 *  Choices for set_modem_config() for a selected subset of common
 *  data rates. If you need another configuration,
 *  determine the necessary settings and call setModemRegisters() with your
 *  desired settings.
 *  Caution: if you are using slow packet rates and long packets with 
 *  RHReliableDatagram or subclasses you may need to change the
 *  RHReliableDatagram timeout for reliable operations. 
 *  Caution: for some slow rates and with ReliableDatagrams you may need to 
 *  increase the reply timeout with manager.setTimeout() to deal with the long 
 *  transmission times.
 */

typedef enum {
    Bw125Cr45Sf128 = 0, ///< Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on. Default medium range
    Bw500Cr45Sf128, ///< Bw = 500 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on. Fast+short range
    Bw31_25Cr48Sf512, ///< Bw = 31.25 kHz, Cr = 4/8, Sf = 512chips/symbol, CRC on. Slow+long range
    Bw125Cr48Sf4096, ///< Bw = 125 kHz, Cr = 4/8, Sf = 4096chips/symbol, CRC on. Slow+long range
} modem_config_choice;

typedef struct LoRa_Packet_Hdr {
    uint8_t to_header;
    uint8_t from_header;
    uint8_t header_id;
    uint8_t header_flags;
} LoRa_Packet_Hdr_t;

typedef struct LoRa_Packet {
    uint8_t preamble;
    LoRa_Packet_Hdr_t header;
    uint8_t *payload;
    uint8_t payload_len;
} LoRa_Packet_t;

#endif	/* RADIO_CONFIG_H */

