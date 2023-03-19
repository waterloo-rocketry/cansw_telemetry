#ifndef ADC_H
#define ADC_H

#include <stdint.h>

void adc_init();

uint16_t read_adc_raw(uint8_t channel);
uint16_t read_adc_mv(uint8_t channel);
uint16_t read_radio_curr_ma();

void update_sensor_low_pass(void);
uint16_t read_radio_curr_low_pass_ma();

#endif	/* ADC_H */