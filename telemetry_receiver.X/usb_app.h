#ifndef USB_APP_H
#define	USB_APP_H

#include <stdint.h>
#include <stdbool.h>

void usb_app_heartbeat(void);

uint8_t usb_app_available_bytes(void);
uint8_t usb_app_read_bytes(char *buffer, uint8_t len);
bool usb_app_write_string(char *buffer, uint8_t len);
//bool usb_app_write_to_console(char *buffer, uint8_t len);

#endif	/* USB_APP_H */

