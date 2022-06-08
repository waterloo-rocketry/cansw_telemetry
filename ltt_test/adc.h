#ifndef ADC_H
#define ADC_H

#include <stdint.h>

void adc_init();

uint16_t read_adc_raw(uint8_t channel);
uint16_t read_adc_mv(uint8_t channel);
uint16_t read_bus_curr_ma();
uint16_t read_batt_curr_ma();
uint16_t read_batt_volt_mv();

#endif	/* ADC_H */