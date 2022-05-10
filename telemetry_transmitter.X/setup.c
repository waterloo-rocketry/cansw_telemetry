#include "setup.h"

void init_pins(void)
{
    // starting with port A
    ANSELA = (0 << 7 | // OSC1 pin
              0 << 6 | // OSC2 pin
              0 << 5 | // BUS_EN_5V
              0 << 4 | // BUS_EN_12V
              0 << 3 | // BUS_EN_GND
              1 << 2 | // Battery VSense
              1 << 1 | // curr_battery line
              1 << 0); // curr_bus line
    LATA = (0 << 7 |   // OSC1 pin
            0 << 6 |   // OSC2 pin
            0 << 5 |   // BUS_EN_5V
            0 << 4 |   // BUS_EN_12V
            0 << 3 |   // BUS_EN_GND
            0 << 2 |   // Battery VSense
            0 << 1 |   // curr_battery line
            0 << 0);   // curr_bus line
    TRISA = (1 << 7 |  // OSC1 pin
             1 << 6 |  // OSC2 pin
             0 << 5 |  // BUS_EN_5V
             0 << 4 |  // BUS_EN_12V
             0 << 3 |  // BUS_EN_GND
             1 << 2 |  // Battery VSense
             1 << 1 |  // curr_battery line
             1 << 0);  // curr_bus line
    // now port B
    ANSELB = (1 << 7 | // ICSPDAT (otherwise unused)
              1 << 6 | // ICSPCLK (otherwise unused)
              1 << 5 | // NC pin
              0 << 4 | // UART_RX
              0 << 3 | // UART_TX
              0 << 2 | //~UART_RTS
              0 << 1 | //~UART_CTS
              1 << 0); // NC pin
    LATB = (0 << 7 |   // ICSPDAT (otherwise unused)
            0 << 6 |   // ICSPCLK (otherwise unused)
            0 << 5 |   // NC pin
            0 << 4 |   // UART_RX
            0 << 3 |   // UART_TX
            0 << 2 |   //~UART_RTS
            0 << 1 |   //~UART_CTS
            0 << 0);   // NC pin
    TRISB = (1 << 7 |  // ICSPDAT (otherwise unused)
             1 << 6 |  // ICSPCLK (otherwise unused)
             1 << 5 |  // NC pin
             0 << 4 |  // UART_RX
             1 << 3 |  // UART_TX
             0 << 2 |  //~UART_RTS
             1 << 1 |  //~UART_CTS
             1 << 0);  // NC pin
    // now port C
    ANSELC = (0 << 7 | // NC
              0 << 6 | // NC
              0 << 5 | // NC
              0 << 4 | // BLUE LED
              0 << 3 | // RED LED
              0 << 2 | // WHITE LED
              0 << 1 | // CAN_RX
              0 << 0); // CAN_TX
    LATC = (0 << 7 |   // NC
            0 << 6 |   // NC
            0 << 5 |   // NC
            0 << 4 |   // BLUE LED
            0 << 3 |   // RED LED
            0 << 2 |   // WHITE LED
            0 << 1 |   // CAN_RX
            0 << 0);   // CAN_TX
    TRISC = (0 << 7 |  // NC
             0 << 6 |  // NC
             0 << 5 |  // NC
             0 << 4 |  // BLUE LED
             0 << 3 |  // RED LED
             0 << 2 |  // WHITE LED
             1 << 1 |  // CAN_RX
             0 << 0);  // CAN_TX

    // setup CAN output pins
    // CANRX on RC1
    CANRXPPS = 0b010001;

    // CANTX on RC0
    RC0PPS = 0b110100;
}

void init_leds(void)
{
    // White LED Setup
    TRISC2 = 0;  // Set white LED as output
    ANSELC2 = 0; // Enable digitali nput buffer
    LATC2 = 1;   // Turn white LED off

    // Red LED Setup
    TRISC3 = 0;  // Set red LED as output
    ANSELC3 = 0; // Enable digital input buffer
    LATC3 = 1;   // Turn red LED off

    // Blue LED Setup
    TRISC4 = 0;  // Set blue LED as output
    ANSELC4 = 0; // Enable digital input buffer
    LATC4 = 1;   // Turn blue LED off
}

void init_osc(void)
{
    // Select external oscillator with PLL of 1:1
    OSCCON1 = 0b01110000;
    // wait until the clock switch has happened
    while (OSCCON3bits.ORDY == 0)
    {
    }
    // if the currently active clock (CON2) isn't the selected clock (CON1)
    if (OSCCON2 != 0b01110000)
    {
        // infinite loop, something is broken, what even is an assert()?
        while (1)
        {
        }
    }
}

void init_adc(void)
{
    // enable fixed voltage reference
    FVRCONbits.EN = 1;
    // set the voltage reference to be 4.096V on both outputs
    FVRCONbits.CDAFVR = 0b11;
    FVRCONbits.ADFVR = 0b11;
    // disable the temperature indicator.... for now....
    FVRCONbits.TSEN = 0;
    // wait for the FVR to stabilize
    while (FVRCONbits.RDY == 0)
    {
    }

    // turn on the ADC
    ADCON0bits.ON = 1;

    // disable repeating operations
    //  if this is set to 1, as soon as ADC finished a reading it starts another
    ADCON0bits.CONT = 0;

    // use system clock as ADC timer. Divider set by ADCLK
    ADCON0bits.CS = 0;

    // this assumes FOSC is 12MHz this value sets ADC clock period to
    //  1.5uS (Fosc/16). Before you change this number, please check
    //  datasheet table 37-1
    ADCLK = 0b000111;

    // right justify the 12 bit output of the ADC.  if this value is 0,
    //  the top 8 bits of read value are put in ADRESH, and the bottom
    //  4 bits are put in the top 4 bits of ADRESL. In this mode,
    //  bottom 8 bits are in ADRESL, top 4 are in bottom 4 bits of
    //  ADRESH.
    ADCON0bits.FM = 1;

    // set the references
    //  negative reference is ground
    ADREFbits.NREF = 0; // 1 would set to external Vref-
    // positive reference to internal FVR module
    ADREFbits.PREF = 0b11;
}

void init_interrupts(void)
{
    // Enable global interrupts
    INTCON0bits.GIE = 1;

    // disable interrupt priorities. Another thing we could be fancy about
    INTCON0bits.IPEN = 0;

    // enable ADC interrupt
    PIE1bits.ADIE = 1;

    // enable timer 0 interrupt
    PIE3bits.TMR0IE = 1;
}
