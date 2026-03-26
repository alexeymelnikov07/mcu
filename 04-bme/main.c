#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "stdio.h"
#include "stdlib.h"
#include "pico/stdlib.h"
#include "stdio-task/stdio-task.h"
#include "protocol-task.h"
#include "led-task/led-task.h"
#include "hardware/i2c.h"

#define DEVICE_NAME "my-pico-device"
#define DEVICE_VRSN "v0.0.1"

uint32_t global_variable = 0;

const uint32_t constant_variable = 42;

// I2C func
void rp2040_i2c_read(uint8_t* buffer, uint16_t length)
{
	i2c_read_timeout_us(i2c1, 0x76, buffer, length, false, 100000);
}
void rp2040_i2c_write(uint8_t* data, uint16_t size)
{
	i2c_write_timeout_us(i2c1, 0x76, data, size, false, 100000);
}

// Callbacks
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

void read_reg_callback(const char* args){
    uint8_t addr;
    uint8_t N;
    
    unsigned int a, n;
    sscanf(args, "%x %x", &a, &n);
    addr = (uint8_t)a;
    N = (uint8_t)n;
    
    
    uint8_t buffer[256] = {0};
    if (addr<=0xFF && N<=0xFF && addr+N<=0xFF) {
           bme280_read_regs(addr, buffer, N);
    }
    else{
        printf("Wrong args\n");
        return;
    }
    for (int i = 0; i < N; i++)
    {
        // printf("addr=%u, N=%x\n",addr+i, N);
        printf("bme280 register [0x%X] = 0x%x\n", addr + i, buffer[i]);
    }
}

void write_reg_callback(const char* args){
    uint8_t addr;
    uint8_t value;

    unsigned int a, v;
    sscanf(args, "%x %u", &a, &v);
    addr = (uint8_t)a;
    value = (uint8_t)v;
    if (addr<=0xFF && value<=32){
        bme280_write_reg(addr,value);
    }
    else printf("Wrong args\n");
}

//---------- Температура, давление, влажность ----------

void temp_raw_callback(const char* args){
    uint16_t adc = bme280_read_temp_raw();
    printf("Temperature = %u [Counts]", adc);
    printf("Temperature = %s [DegC]", BME280_compensate_T_int32((uint32_t)adc, bme280_calib)/100);
}

void pres_raw_callback(const char* args){
    uint16_t adc = bme280_read_pres_raw();
    printf("Preasure = %u [Counts]", adc);
    printf("Preasure = %u [Pa]", BME280_compensate_P_int64((uint32_t)adc, bme280_calib)/256);
}

void hum_raw_callback(const char* args){
    uint16_t adc = bme280_read_hum_raw();
    printf("Humidity = %u [counts]", adc);
    printf("Humidity = %u [Percentage]", bme280_compensate_H_int32((uint32_t)adc, bme280_calib)/1024);
}

api_t device_api[] =
{
	{"version", version_callback, "get device name and firmware version"},
    {"on",led_on_callback,"turn on the led"},
    {"off",led_off_callback,"turn off the led"},
    {"blink",led_blink_callback,"make lead blink"},
    {"read_reg", read_reg_callback, ""},
    {"write_reg", write_reg_callback, ""},
    {"temp_raw", temp_raw_callback, ""},
    {"pres_raw", pres_raw_callback, ""},
    {"hum_raw", hum_raw_callback, ""},
	{NULL, NULL, NULL},
};

int main(){
    stdio_init_all();
    stdio_task_init();
    protocol_task_init(device_api);
    led_task_init();

    i2c_init(i2c1, 100000);
    gpio_set_function(14, GPIO_FUNC_I2C);
    gpio_set_function(15, GPIO_FUNC_I2C);
    bme280_init(rp2040_i2c_read, rp2040_i2c_write);

    while(1){
        protocol_task_handle(stdio_task_handle());
        led_task_handle();
    }
}