#ifndef ADC_H
#define ADC_H

#include <stdint.h>

void adc_init();

uint16_t read_adc_raw(uint8_t channel);
uint16_t read_adc_mv(uint8_t channel);
uint16_t read_bus_curr_ma();
uint16_t read_batt_curr_ma();
uint16_t read_batt_volt_mv();

void update_sensor_low_pass(void);
uint16_t read_bus_curr_low_pass_ma();
uint16_t read_batt_curr_low_pass_ma();
uint16_t read_batt_volt_low_pass_mv();

#endif	/* ADC_H */