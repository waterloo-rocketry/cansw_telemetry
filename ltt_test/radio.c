#include "radio.h"
#include <stdint.h>
#include "canlib.h"
#include "uart.h"
#include <xc.h>

// crc table used for calculation
uint8_t table[256] = {
    0x00, 0x07, 0x0e, 0x09, 0x1c, 0x1b, 0x12, 0x15, 0x38, 0x3f, 0x36, 0x31, 0x24, 0x23, 0x2a, 0x2d,
    0x70, 0x77, 0x7e, 0x79, 0x6c, 0x6b, 0x62, 0x65, 0x48, 0x4f, 0x46, 0x41, 0x54, 0x53, 0x5a, 0x5d,
    0xe0, 0xe7, 0xee, 0xe9, 0xfc, 0xfb, 0xf2, 0xf5, 0xd8, 0xdf, 0xd6, 0xd1, 0xc4, 0xc3, 0xca, 0xcd,
    0x90, 0x97, 0x9e, 0x99, 0x8c, 0x8b, 0x82, 0x85, 0xa8, 0xaf, 0xa6, 0xa1, 0xb4, 0xb3, 0xba, 0xbd,
    0xc7, 0xc0, 0xc9, 0xce, 0xdb, 0xdc, 0xd5, 0xd2, 0xff, 0xf8, 0xf1, 0xf6, 0xe3, 0xe4, 0xed, 0xea,
    0xb7, 0xb0, 0xb9, 0xbe, 0xab, 0xac, 0xa5, 0xa2, 0x8f, 0x88, 0x81, 0x86, 0x93, 0x94, 0x9d, 0x9a,
    0x27, 0x20, 0x29, 0x2e, 0x3b, 0x3c, 0x35, 0x32, 0x1f, 0x18, 0x11, 0x16, 0x03, 0x04, 0x0d, 0x0a,
    0x57, 0x50, 0x59, 0x5e, 0x4b, 0x4c, 0x45, 0x42, 0x6f, 0x68, 0x61, 0x66, 0x73, 0x74, 0x7d, 0x7a,
    0x89, 0x8e, 0x87, 0x80, 0x95, 0x92, 0x9b, 0x9c, 0xb1, 0xb6, 0xbf, 0xb8, 0xad, 0xaa, 0xa3, 0xa4,
    0xf9, 0xfe, 0xf7, 0xf0, 0xe5, 0xe2, 0xeb, 0xec, 0xc1, 0xc6, 0xcf, 0xc8, 0xdd, 0xda, 0xd3, 0xd4,
    0x69, 0x6e, 0x67, 0x60, 0x75, 0x72, 0x7b, 0x7c, 0x51, 0x56, 0x5f, 0x58, 0x4d, 0x4a, 0x43, 0x44,
    0x19, 0x1e, 0x17, 0x10, 0x05, 0x02, 0x0b, 0x0c, 0x21, 0x26, 0x2f, 0x28, 0x3d, 0x3a, 0x33, 0x34,
    0x4e, 0x49, 0x40, 0x47, 0x52, 0x55, 0x5c, 0x5b, 0x76, 0x71, 0x78, 0x7f, 0x6a, 0x6d, 0x64, 0x63,
    0x3e, 0x39, 0x30, 0x37, 0x22, 0x25, 0x2c, 0x2b, 0x06, 0x01, 0x08, 0x0f, 0x1a, 0x1d, 0x14, 0x13,
    0xae, 0xa9, 0xa0, 0xa7, 0xb2, 0xb5, 0xbc, 0xbb, 0x96, 0x91, 0x98, 0x9f, 0x8a, 0x8d, 0x84, 0x83,
    0xde, 0xd9, 0xd0, 0xd7, 0xc2, 0xc5, 0xcc, 0xcb, 0xe6, 0xe1, 0xe8, 0xef, 0xfa, 0xfd, 0xf4, 0xf3
};

uint8_t crc8_checksum(uint8_t *pdata, size_t nbytes, uint8_t crc) {
    while (nbytes > 0) {
        crc = table[(crc ^ *pdata++) & 0xff];
        --nbytes;
    }
    return crc;
}

uint8_t hex2num(char ch) {
    if (ch >= '0' && ch <= '9')
        return ch - '0';
    if (ch >= 'A' && ch <= 'F')
        return ch - 'A' + 10;
    if (ch >= 'a' && ch <= 'f')
        return ch - 'a' + 10;
    return 255;
}

typedef enum {
    WAITING,
    SID,
    DATA_SEP,
    DATA,
    CHECKSUM,
} parse_state_t;

void radio_handle_input_character(char c) {
    static can_msg_t msg;
    static uint8_t msg_crc;
    static parse_state_t parse_state = WAITING;
    static uint8_t parse_i = 0;

    uint8_t d;
    switch(parse_state) {
        case WAITING:
            if(c == 'm' || c == 'M') {
                msg.sid = 0;
                msg.data_len = 0;
                parse_state = SID;
                parse_i = 0;
            }
            break;

        case SID:
            d = hex2num(c);

            if(d == 255) { // invalid hex
                parse_state = WAITING;
                break;
            }

            msg.sid = (msg.sid << 4 | d) & 0x0fff;

            parse_i++;

            if(parse_i == 1) {
                // checksum for first sid byte
                msg_crc = crc8_checksum(&d, 1, 0);
            }

            if(parse_i >= 3) { // sid has 3 hex digits
                // checksum for second sid byte
                // passing uint16_t* as uint8_t* in little endian results in lowest byte only
                msg_crc = crc8_checksum((uint8_t*) &msg.sid, 1, msg_crc);
                parse_state = DATA_SEP;
            }

            break;

        case DATA:
            d = hex2num(c);

            if(d == 255) { // invalid hex
                parse_state = WAITING;
                break;
            }

            msg.data[msg.data_len-1] = (msg.data[msg.data_len-1] << 4 | d) & 0xff;

            if(++parse_i >= 2) { // DATA bytes have 2 hex digits
                // checksum for this data byte
                msg_crc = crc8_checksum(&d, 1, msg_crc);
                parse_state = DATA_SEP;
            }

            break;

        case CHECKSUM:
            d = hex2num(c);

            if (d == 255) { // invalid hex
                parse_state = WAITING;
                return;
            }

            static uint8_t exp_crc;
            exp_crc = (exp_crc << 4 | d) & 0xff;

            if(++parse_i == 2) { // crc8 has 2 hex digits
                if(exp_crc == msg_crc) {
                    if (get_message_type(&msg) == MSG_RESET_CMD && get_reset_board_id(&msg) == 0) {
                        RESET();
                    }
                    txb_enqueue(&msg);
                }

                parse_state = WAITING;
            }
            break;

        case DATA_SEP:
            switch(c) {
                case ',':
                    parse_state = DATA;
                    parse_i = 0;
                    if(msg.data_len < 8)
                        msg.data[msg.data_len++] = 0;
                    else // too much data
                        parse_state = WAITING;
                    break;
                case ';':
                    parse_state = CHECKSUM;
                    parse_i = 0;
                    break;
                default:
                    parse_state = WAITING;
                    break;
            }
            break;

        default:
            parse_state = WAITING;
            break;
    }
}

void serialize_can_msg(can_msg_t *msg) {
    /*
       |-------------+------------+--------+-----------------------------------|
       | byte[bit]   | name       | length | comment                           |
       |-------------+------------+--------+-----------------------------------|
       | 0           | header     | 1 byte |                                   |
       | 0[7..6]     | sync start | 2 bit  | always 0x2, marks start of frame  |
       | 0[5..0]     | CRC LSB    | 6 bit  | bit [5..0] of CRC                 |
       |-------------+------------+--------+-----------------------------------|
       | 1..2        | sid        | 2 byte | lower byte first                  |
       | bit[7]      | reserved   | 1 bit  | MSB = 0 to indicate frame content |
       | bit[6..0]   | sid        | 7 bit  | bits [13..7], [6..0] of sid       |
       |-------------+------------+--------+-----------------------------------|
       | 3..length+2 | data       | length |                                   |
       | bit[7]      | reserved   | 1 bit  | MSB = 0 to indicate frame content |
       | bit[6..0]   | data       | 7 bit  | bits [6..0] of data bytes         |
       |-------------+------------+--------+-----------------------------------|
       | length+3    | MSBs       | 1 byte |                                   |
       | bit[7]      | reserved   | 1 bit  | MSB = 0 to indicate frame content |
       | bit[6..5]   | sid        | 2 bit  | bit [15..14] of sid               |
       | bit[4..0]   | data[4..0] | 5 bit  | bit 7 for data [4..0]             |
       |-------------+------------+--------+-----------------------------------|
       | length+4    | frame end  | 1 byte | end of frame                      |
       | bit[7..6]   | sync end   | 2 bit  | always 0x3, marks end of frame    |
       | bit[5]      | unused     | 1 bit  |                                   |
       | bit[4..3]   | CRC MSB    | 2 bit  | bit [7..6] of CRC                 |
       | bit[2..0]   | data[7..5] | 3 bit  | bit 7 for data [7..5]             |
       |-------------+------------+--------+-----------------------------------|
       CRC is calculated from {sid & 0xFF, sid >> 8 & 0xFF, data}
   */
    if(msg->data_len == 0) return;
    uint8_t len = msg->data_len < 8 ? msg->data_len : 8;
    uint8_t buff[13] = {0};

    // CRC calculation
    uint8_t crc = 0;
    uint8_t sid_l = msg->sid      & 0xff;
    uint8_t sid_h = msg->sid >> 8 & 0xff;
    crc = crc8_checksum(&sid_l, 1, crc);
    crc = crc8_checksum(&sid_h, 1, crc);
    crc = crc8_checksum(msg->data, len, crc);

    // header, frame end, CRC
    buff[0]     = 0x80 | (crc & 0x3f);
    buff[len+4] = 0xC0 | (crc & 0xc0) >> 3;

    // sid
    buff[1] = msg->sid      & 0x7f;
    buff[2] = msg->sid >> 7 & 0x7f;
    buff[len+3] = (msg->sid & 0xC000) >> 9;

    // data
    for(uint8_t i = 0; i < len; i++) {
        buff[i+3]              =  msg->data[i] & 0x7f;
        if(i < 5) buff[len+3] |= (msg->data[i] & 0x80) >> (7-i);
        else      buff[len+4] |= (msg->data[i] & 0x80) >> (7-i+5);
    }

    // transmit
    uart_transmit_buffer(buff, len+5);
}
