#include "radio.h"
#include <stdint.h>
#include "canlib.h"
#include "uart.h"
#include <xc.h>

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
    static can_msg_t msg;
    static uint8_t EoM_flag = 0;
    static uint8_t sum1 = 0;
    static uint8_t sum2 = 0;
    if (parse_i == 0) { // expecting the start of a new message
        if (c == 'm' || c == 'M') {
            msg.sid = 0;
            msg.data_len = 0;
            EoM_flag = 0; // not the end of a message
            sum1 = 0;
            sum2 = 0;
            parse_i++;
        } else {} // ignore unknown character
        return;
    } else if (parse_i <= 3) { // SID bits
        uint8_t d = hex2num(c);
        if (d == 255) { // invalid character
            parse_i = 0;
            return;
        }
        sum1 = (sum1 + d) % 15; // This should be fine mathmatically, since sum1 and d < 16
        sum2 = (sum1 + sum2) % 15;
        msg.sid |= d << ((3 - parse_i) * 4);
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
        if (EoM_flag){
            uint8_t exp_sum = d;
            //compare checksum b4 send
            if (exp_sum == (sum1 ^ sum2)) {
                if (get_message_type(&msg) == MSG_RESET_CMD && get_reset_board_id(&msg) == 0) {
                    //SET_BUS_POWER(false);
                    RESET();
                }
                txb_enqueue(&msg);
            }
            parse_i = 0;
            return; 
        }
        sum1 = (sum1 + d) % 15; // This should be fine mathmatically, since sum1 and d < 16
        sum2 = (sum1 + sum2) % 15;
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
    
    //length for 1 byte ("XX,") * number of bytes-1 + extras and last byte
    char temp_buffer[5 + 3 * 8 + 2];
    // The first character is to identify this as a valid message
    // The next 3 characters are the sid of the input message followed by a ':'
    // The next 2 characters are elements of the data array apart of the input message followed by a ','
    // The next 2 characters are for formatting output
    // The last character is used to end the string made from temp_buffer
    uint8_t sum1 = 0;
    uint8_t sum2 = 0;
    temp_buffer[0] = '$';
    temp_buffer[1] = hex_lookup_table[(msg->sid >> 8) & 0xf];
    sum1 = (sum1 + ((msg->sid >> 8) & 0xf)) % 15;
    sum2 = (sum1 + sum2) % 15;
    temp_buffer[2] = hex_lookup_table[(msg->sid >> 4) & 0xf];
    sum1 = (sum1 + ((msg->sid >> 4) & 0xf)) % 15;
    sum2 = (sum1 + sum2) % 15;
    temp_buffer[3] = hex_lookup_table[msg->sid & 0xf];
    sum1 = (sum1 + (msg->sid & 0xf)) % 15;
    sum2 = (sum1 + sum2) % 15;
    temp_buffer[4] = ':';
    uint8_t i = 0;
    for (i = 0; i < msg->data_len && i < 8; ++i) {
        temp_buffer[3 * i + 5] = hex_lookup_table[(msg->data[i] >> 4)];
        sum1 = (sum1 + (msg->data[i] >> 4)) % 15;
        sum2 = (sum1 + sum2) % 15;
        temp_buffer[3 * i + 6] = hex_lookup_table[(msg->data[i] & 0xf)];
        sum1 = (sum1 + (msg->data[i] & 0xf)) % 15;
        sum2 = (sum1 + sum2) % 15;
        temp_buffer[3 * i + 7] = ',';
    }
    temp_buffer[3 * i + 4] = ';'; // Delimit for checksum
    temp_buffer[3 * i + 5] = hex_lookup_table[(sum1 ^ sum2)];
    // Fletcher's checksum augmented for 4 bit & using XOR at the end. See wikipedia for details.
    temp_buffer[3 * i + 6] = '\n';
    uart_transmit_buffer(temp_buffer, 3 * i + 7);
}
