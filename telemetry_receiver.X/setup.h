#ifndef SETUP_H
#define	SETUP_H

#include <xc.h>

// LEDs
// This is for LT Receiver
#define WHITE_LED_ON() (LATA4 = 0)
#define WHITE_LED_OFF() (LATA4 = 1)
#define RED_LED_ON() (LATA5 = 0)
#define RED_LED_OFF() (LATA5 = 1)
#define BLUE_LED_ON() (LATC5 = 0)
#define BLUE_LED_OFF() (LATC5 = 1)

/*
// This is for Keto for testing
#define WHITE_LED_ON() (LATB4 = 0)
#define WHITE_LED_OFF() (LATB4 = 1)
#define RED_LED_ON() (LATB3 = 0)
#define RED_LED_OFF() (LATB3 = 1)
#define BLUE_LED_ON() (LATB2 = 0)
#define BLUE_LED_OFF() (LATB2 = 1)
 */

void init_leds(void);

void LED_HEARTBEAT(void);

#endif	/* SETUP_H */

