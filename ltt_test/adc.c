#include "adc.h"
#include <xc.h>

void adc_init() {
    ANSELA = 0b00000111;
    ANSELB = 0;
    ANSELC = 0;

    // Configure FVR module
    // b[7] enables FVR
    // b[1:0] sets the reference to 2.048
    FVRCON = 0b10000010;

    // Configure ADC module
    ADCON0bits.FM = 1; // Right justify
    ADCON0bits.ADCONT = 0; // Not continuous
    ADCON0bits.ADCS = 0; // Drive from Fosc...
    ADCLKbits.CS = 0b111111; // ...Divided by 128
    ADCON2bits.MD = 0; // Basic mode (no fancy calculations)
    ADREFbits.NREF = 0; // Negative reference is GND
    ADREFbits.PREF = 0b11; // Positive reference is FVR
}

uint16_t read_adc_raw(uint8_t channel) {
    ADPCH = channel;
    ADCON0bits.ADON = 1;
    ADCON0bits.ADGO = 1;
    while (ADCON0bits.ADGO);
    return (ADRESH << 8) | ADRESL;
}

uint16_t read_adc_mv(uint8_t channel) {
    return read_adc_raw(channel) / 2; // 2.048 ref, 12 bit ADC
}

uint16_t read_bus_curr_ma() {
    return read_adc_mv(0) / 2; // 100 V/V amplification, 20mR sense
}

uint16_t read_batt_curr_ma() {
    return read_adc_mv(1) / 2; // 100 V/V amplification, 20mR sense
}

uint16_t read_batt_volt_mv() {
    return read_adc_raw(2) * 16 / 5; // raw is in 0.5 mV, 6.4 resistor divider
}
