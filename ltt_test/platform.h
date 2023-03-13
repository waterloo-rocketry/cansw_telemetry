#ifndef BOARD_H
#define	BOARD_H

#include <stdbool.h>

void gpio_init(void);

void RED_LED_SET(bool value);
void BLUE_LED_SET(bool value);
void WHITE_LED_SET(bool value);

void SET_RADIO_POWER(bool value);

#endif	/* BOARD_H */

