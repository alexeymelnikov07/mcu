#pragma once

#include <stdint.h>

typedef void (*bme280_i2c_read)(uint8_t* buffer, uint16_t length);
typedef void (*bme280_i2c_write)(uint8_t* data, uint16_t size);

void bme280_init(bme280_i2c_read i2c_read, bme280_i2c_write i2c_write);
void bme280_read_regs(uint8_t start_reg_address, uint8_t* buffer, uint8_t length);
void bme280_write_reg(uint8_t reg_address, uint8_t value);
uint16_t bme280_read_temp_raw();
uint16_t bme280_read_pres_raw();
uint16_t bme280_read_hum_raw();

// регистры

#define BME280_RED_id 0xD0
#define BME280_RED_reset 0xE0
#define BME280_RED_ctrl_hum 0xF2
#define BME280_RED_status 0xF3
#define BME280_RED_ctrl_meas 0xF4 
#define BME280_RED_config 0xF5 
#define BME280_RED_press_msb 0xF7
#define BME280_RED_press_lsb 0xF8 
#define BME280_RED_press_xlsb 0xF9
#define BME280_RED_temp_msb 0xFA
#define BME280_RED_temp_lsb 0xFB
#define BME280_RED_temp_xlsb 0xFC
#define BME280_RED_hum_msb 0xFD
#define BME280_RED_hum_lsb 0xFE

typedef struct {
    uint16_t dig_T1;
    int16_t  dig_T2;
    int16_t  dig_T3;
    
    uint16_t dig_P1;
    int16_t  dig_P2;
    int16_t  dig_P3;
    int16_t  dig_P4;
    int16_t  dig_P5;
    int16_t  dig_P6;
    int16_t  dig_P7;
    int16_t  dig_P8;
    int16_t  dig_P9;
    
    uint8_t  dig_H1;
    int16_t  dig_H2;
    uint8_t  dig_H3;
    int16_t  dig_H4;
    int16_t  dig_H5;
    uint8_t  dig_H6;
} bme280_calib_data;

void bme280_read_calibration(bme280_calib_data *cal);