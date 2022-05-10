#ifndef SETUP_H
#define SETUP_H

#include <xc.h>

#define _XTAL_FREQ 12000000 // 12MHz

// LEDs
// This is for LT Transmitter
#define WHITE_LED_ON() \
    do                 \
    {                  \
        LATC2 = 0;     \
    } while (0)
#define WHITE_LED_OFF() \
    do                  \
    {                   \
        LATC2 = 1;      \
    } while (0)
#define RED_LED_ON() \
    do               \
    {                \
        LATC3 = 0;   \
    } while (0)
#define RED_LED_OFF() \
    do                \
    {                 \
        LATC3 = 1;    \
    } while (0)
#define BLUE_LED_ON() \
    do                \
    {                 \
        LATC4 = 0;    \
    } while (0)
#define BLUE_LED_OFF() \
    do                 \
    {                  \
        LATC4 = 1;     \
    } while (0)

#define BUS_POWER_ON() \
    do                 \
    {                  \
        LATA2 = 1;     \
    } while (0)
#define BUS_POWER_OFF() \
    do                  \
    {                   \
        LATA2 = 0;      \
    } while (0)

// macros that define what pins are connected to what
#define ANALOG_CH_BATT_VOLTAGE 3
#define ANALOG_CH_BATT_CURRENT 1
#define ANALOG_CH_BUS_CURRENT 0

void init_leds(void);

void init_pins(void);

void init_osc(void);

void init_adc(void);

void init_interrupts(void);

#endif /* SETUP_H */
