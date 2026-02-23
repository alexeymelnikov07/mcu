#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "stdio.h"
#include "stdlib.h"
#include "pico/stdlib.h"
#include "stdio-task/stdio-task.h"
#include "protocol-task/protocol-task.h"
#include "led-task/led-task.h"
#include "adc-task/adc-task.h"

#define DEVICE_NAME "my-pico-device"
#define DEVICE_VRSN "v0.0.1"

uint32_t global_variable = 0;

//const uint LED_PIN = 25;
const uint32_t constant_variable = 42;

void version_callback(const char* args)
{
	printf("device name: '%s', firmware version: %s\n", DEVICE_NAME, DEVICE_VRSN);
}

void led_on_callback(const char* args){
    led_task_state_set(LED_STATE_ON);
}
void led_off_callback(const char* args){
    led_task_state_set(LED_STATE_OFF);
}
void led_blink_callback(const char* args){
    led_task_state_set(LED_STATE_BLINK);
}
void get_adc_callback(const char* args){
    printf("%f\n", adc_task_take_voltage());
}
void get_temp_callback(const char * args){
    printf("%f\n", adc_task_take_temp());
}
void tm_start_callback(const char* args){
    adc_task_set_state(ADC_TASK_STATE_RUN);
}
void tm_stop_callback(const char* args){
    adc_task_set_state(ADC_TASK_STATE_IDLE);
}
api_t device_api[] =
{
	{"version", version_callback, "get device name and firmware version"},
    {"on",led_on_callback,""},
    {"off",led_off_callback,""},
    {"blink",led_blink_callback,""},
    {"get_adc",get_adc_callback,""},
    {"get_temp",get_temp_callback, ""},
    {"tm_start",tm_start_callback, ""},
    {"tm_stop",tm_stop_callback, ""},
	{NULL, NULL, NULL},
};

int main(){
    stdio_init_all();
    stdio_task_init();
    protocol_task_init(device_api);
    led_task_init();
    adc_task_init();
    while(1){
        protocol_task_handle(stdio_task_handle());
        led_task_handle();
        adc_task_handle();
    }
}