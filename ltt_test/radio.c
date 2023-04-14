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

void radio_handle_input_character(char c) {
    static uint8_t parse_i = 0;
    static uint8_t crc_i = 0;
    static can_msg_t msg;
    static uint8_t EoM_flag = 0;
    if (parse_i == 0) { // expecting the start of a new message
        if (c == 'm' || c == 'M') {
            msg.sid = 0;
            msg.data_len = 0;
            crc_i = 0;
            parse_i++;
        } else {
        } // ignore unknown character
        return;
    } else if (parse_i <= 3) { // SID bits
        uint8_t d = hex2num(c);
        if (d == 255) { // invalid character
            parse_i = 0;
            return;
        }
        msg.sid |= (uint16_t) (d << ((3 - parse_i) * 4));
        parse_i++;
        return;
    } else if (parse_i % 3 == 1) { // We expect a comma or a semicolon
        if (c == ',') { // another data byte follows, make room for it
            msg.data_len += 1;
            msg.data[msg.data_len - 1] = 0;
            parse_i++;
            return;
        }
        if (c == ';') { // end of message
            EoM_flag = 1;
            return;
        }
        // either the message ended or it was an invalid character, either way reset
        parse_i++; // go to the if (EoM_flag) below next parse 
        return;
    } else { // hex data chars
        uint8_t d = hex2num(c);
        if (d == 255) { // invalid character
            parse_i = 0;
            EoM_flag = 0;
            return;
        }
        if (EoM_flag) {
            static uint8_t exp_crc;
            if (crc_i == 0) {
                exp_crc = d;
                crc_i++;
            }
            if (crc_i == 1) {
                exp_crc = (exp_crc << 4) | d;
                
                uint8_t crc  = table[(0x00 ^ ((msg.sid) >> 8)) & 0xff]; // sid 1st byte
                crc  = table[(crc ^ ((msg.sid) & 0xff)) & 0xff];               // sid 2nd byte
                crc = crc8_checksum(msg.data, msg.data_len, crc);

                //compare expect crc with calculated
                if (exp_crc == crc) {
                    if (get_message_type(&msg) == MSG_RESET_CMD && get_reset_board_id(&msg) == 0) {
                        //SET_BUS_POWER(false);
                        RESET();
                    }
                    txb_enqueue(&msg);
                }
                parse_i = 0;
            }
            return;
        }
        // parse_i % 3 == 2 for the first nibble and 0 for the second, so just double it
        msg.data[msg.data_len - 1] |= d << ((parse_i % 3) * 2);
        parse_i++;
        return;
    }
}

void serialize_can_msg(can_msg_t *msg) {
    const char hex_lookup_table[16] = {
        '0', '1', '2', '3',
        '4', '5', '6', '7',
        '8', '9', 'A', 'B',
        'C', 'D', 'E', 'F'
    };

    // crc8 for error checking, need 7 parity bits
    //length for 1 byte ("XX,") * number of bytes-1 + extras and last byte
    char temp_buffer[5 + 3 * 8 + 2 + 1];
    // The first character is to identify this as a valid message
    // The next 3 characters are the sid of the input message followed by a ':'
    // The next 2 characters are elements of the data array apart of the input message followed by a ','
    // The next 2 characters are for formatting output
    // The last character is used to end the string made from temp_buffer
    temp_buffer[0] = '$';
    temp_buffer[1] = hex_lookup_table[(msg->sid >> 8) & 0xf];
    temp_buffer[2] = hex_lookup_table[(msg->sid >> 4) & 0xf];
    temp_buffer[3] = hex_lookup_table[msg->sid & 0xf];

    temp_buffer[4] = ':';
    uint8_t i = 0;
    for (i = 0; i < msg->data_len && i < 8; ++i) {
        temp_buffer[3 * i + 5] = hex_lookup_table[(msg->data[i] >> 4)];
        temp_buffer[3 * i + 6] = hex_lookup_table[(msg->data[i] & 0xf)];
        temp_buffer[3 * i + 7] = ',';
    }
    temp_buffer[3 * i + 4] = ';'; // Delimit for checksum

    // calculate crc8
    uint8_t crc  = table[(0x00 ^ ((msg->sid) >> 8)) & 0xff]; // sid 1st byte
    crc  = table[(crc ^ ((msg->sid) & 0xff)) & 0xff];               // sid 2nd byte
    crc = crc8_checksum(msg->data, msg->data_len, crc);

    temp_buffer[3 * i + 5] = hex_lookup_table[(crc >> 4)];
    temp_buffer[3 * i + 6] = hex_lookup_table[(crc & 0xf)];
    temp_buffer[3 * i + 7] = '\n';

    uart_transmit_buffer(temp_buffer, 3 * i + 8);
}
