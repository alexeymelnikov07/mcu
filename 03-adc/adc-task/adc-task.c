#include "adc-task.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "stdio.h"

const uint PIN = 26;
const uint ADC = 0; 
const uint TEMP_ADC = 4;
uint ADC_TASK_MEAS_PERIOD_US=100000;

adc_task_state_t adc_state;
uint64_t adc_ts;

void adc_task_init(){
    adc_init();
    adc_gpio_init(ADC);
    adc_set_temp_sensor_enabled(true);
    adc_state=ADC_TASK_STATE_IDLE;
    adc_ts=0;
}

float adc_task_take_voltage(){
    adc_select_input(ADC);
    uint16_t voltage_counts = adc_read();
    return voltage_counts/4096.0f*3.3f;
}

float adc_task_take_temp(){
    adc_select_input(TEMP_ADC);
    uint16_t temp_counts = adc_read();
    return 27.0f - (temp_counts/4096.0f*3.3f - 0.706f) / 0.001721f;
}

void adc_task_set_state(adc_task_state_t state){
    adc_state = state;
}

void adc_task_handle(){
    switch(adc_state){
        case ADC_TASK_STATE_IDLE:
            break;
        case ADC_TASK_STATE_RUN:
            if (time_us_64()>adc_ts){
                adc_ts=time_us_64()+ADC_TASK_MEAS_PERIOD_US;
                printf("%f %f\n", adc_task_take_voltage(), adc_task_take_temp());
            }
            break;
        default:
            break;
    }
}