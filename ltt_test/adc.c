#include "adc.h"
#include <xc.h>

void adc_init() {
    ANSELA = 0b1000;
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

uint16_t read_radio_curr_ma() {
    return read_adc_mv(4) / 2; // 100 V/V amplification, 20mR sense
}

//zach derived the equation alpha = (Fs*T/5)/ (1 + Fs*T/5)
// where Fs = sampling frequency and T = response time
// response time is equivalent to 5*tau or 5/2pi*Fc, where Fc is cutoff frequency

#define SAMPLE_FREQ 200
#define LOW_PASS_ALPHA(TR) ((SAMPLE_FREQ * TR / 5.0) / (1 + SAMPLE_FREQ * TR / 5.0))
#define LOW_PASS_RESPONSE_TIME 5  //seconds
double alpha_low = LOW_PASS_ALPHA(LOW_PASS_RESPONSE_TIME);
double low_pass_radio_curr_ma = 0;
void update_sensor_low_pass(void) {
    uint16_t new_radio_curr = read_radio_curr_ma();
    low_pass_radio_curr_ma = alpha_low*low_pass_radio_curr_ma + (1.0 - alpha_low)*new_radio_curr;
}

uint16_t read_radio_curr_low_pass_ma() {
    return (uint16_t) low_pass_radio_curr_ma;
}
