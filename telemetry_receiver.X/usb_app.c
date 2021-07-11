#include "usb_app.h"
#include "setup.h"
#include "mcc_generated_files/usb/usb.h"
#include "mcc_generated_files/mcc.h"

#include <xc.h>

#include <string.h>
#include <stdlib.h>
#include <stdint.h>

static uint8_t read_buffer[64];
static uint8_t write_buffer[64];
static uint8_t read_buffer_len = 0;
static uint8_t write_buffer_len = 0;
static bool write_buffer_is_empty = true;

void usb_app_heartbeat(void)
{
    if( USBGetDeviceState() < CONFIGURED_STATE )
    {
        return;
    }

    if( USBIsDeviceSuspended()== true )
    {
        return;
    }

    if( USBUSARTIsTxTrfReady() == true)
    {
//        uint8_t i;
//        uint8_t numBytesRead;
        if(read_buffer_len == 0)
        {
            read_buffer_len = getsUSBUSART(read_buffer, sizeof(read_buffer));
        }

//        for(i=0; i<read_buffer_len; i++)
//        {
//            switch(read_buffer[i])
//            {
//                /* echo line feeds and returns without modification. */
//                case 0x0A:
//                case 0x0D:
//                    write_buffer[i] = read_buffer[i];
//                    write_buffer_is_empty = false;
//                    break;
//
//                /* all other characters get +1 (e.g. 'a' -> 'b') */
//                default:
//                    write_buffer[i] = read_buffer[i] + 1;
//                    write_buffer_is_empty = false;
//                    break;
//            }
//        }
        
        if (!write_buffer_is_empty)
        {
            putsUSBUSART(write_buffer);
            write_buffer_is_empty = true;
        }
//        read_buffer_len = 0;
//        if(numBytesRead > 0)
//        {
//            putUSBUSART(write_buffer,numBytesRead);
//        }
    }
    
    // 
    CDCTxService();
}

uint8_t usb_app_available_bytes(void)
{
    return read_buffer_len;
}

uint8_t usb_app_read_bytes(char *buffer, uint8_t len)
{
    if (read_buffer_len == 0)
    {
        *buffer = '\0';
        return 0;
    }
    if (len <= read_buffer_len)
    {
        *buffer = '\0';
        return 0;
    }
    memcpy(buffer, read_buffer, read_buffer_len);
    buffer[read_buffer_len] = '\0';
    read_buffer_len = 0;
}

bool usb_app_write_string(char *buffer, uint8_t len)
{
    if ( (false == write_buffer_is_empty) || (len >= sizeof(write_buffer)))
    {
        return false;
    }
    memcpy(write_buffer, buffer, len);
    write_buffer[len] = '\0';
    write_buffer_len = len;
    write_buffer_is_empty = false;
    return true;
}

//bool usb_app_write_to_console(char *buffer, uint8_t len)
//{
//    if ( (false == write_buffer_is_empty) || (len >= sizeof(write_buffer)) || USBUSARTIsTxTrfReady() == false)
//    {
//        return false;
//    }
//    
//    RED_LED_ON();
//    
//    memcpy(write_buffer, buffer, len);
//    write_buffer[len] = '\0';
//    putUSBUSART(write_buffer, len);
//    __delay_ms(1000);
//    
//    RED_LED_OFF();
//    
//    write_buffer_is_empty = true;
//    return true;
//}