#ifndef SETUP_H
#define	SETUP_H

#include <xc.h>

// LEDs
// This is for LT Transmitter
#define WHITE_LED_ON() (LATC2 = 0)
#define WHITE_LED_OFF() (LATC2 = 1)
#define RED_LED_ON() (LATC3 = 0)
#define RED_LED_OFF() (LATC3 = 1)
#define BLUE_LED_ON() (LATC4 = 0)
#define BLUE_LED_OFF() (LATC4 = 1)


// This is for Keto for testing
/*#define WHITE_LED_ON() (LATB4 = 0)
#define WHITE_LED_OFF() (LATB4 = 1)
#define RED_LED_ON() (LATB3 = 0)
#define RED_LED_OFF() (LATB3 = 1)
#define BLUE_LED_ON() (LATB2 = 0)
#define BLUE_LED_OFF() (LATB2 = 1)*/


void init_leds(void);

void init_osc(void);

void init_interrupts(void);

#endif	/* SETUP_H */

