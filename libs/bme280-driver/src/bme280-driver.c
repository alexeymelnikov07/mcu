#include "bme280-driver.h"

typedef struct
{
	bme280_i2c_read i2c_read;
	bme280_i2c_write i2c_write;
} bme280_ctx_t;


static bme280_ctx_t bme280_ctx = {0};
bme280_calib_data bme280_calib = {0};


void bme280_init(bme280_i2c_read i2c_read, bme280_i2c_write i2c_write){
    bme280_ctx.i2c_read = i2c_read;
    bme280_ctx.i2c_write = i2c_write;
    uint8_t id_reg_buf[1] = {0};

    bme280_read_regs(BME280_REG_id, id_reg_buf, sizeof(id_reg_buf));
    if (id_reg_buf!=0x60) {printf("ID is not 0x60\n");}

    uint8_t ctrl_hum_reg_value = 0;
    ctrl_hum_reg_value |= (0b001 << 0); // osrs_h[2:0] = oversampling 1
    bme280_write_reg(BME280_REG_ctrl_hum, ctrl_hum_reg_value);

    uint8_t config_reg_value = 0;
    config_reg_value |= (0b0 << 0); // spi3w_en[0:0] = false
    config_reg_value |= (0b000 << 2); // filter[4:2] = Filter off
    config_reg_value |= (0b001 << 5); // t_sb[7:5] = 62.5 ms
    bme280_write_reg(BME280_REG_config, config_reg_value);

    uint8_t ctrl_mead_reg_value = 0;
    ctrl_hum_reg_value |=(0b11 << 0); // mode[1:0]
    ctrl_hum_reg_value |=(0b001 << 2); // osrs_p[4:2] 
    ctrl_hum_reg_value |=(0b001 << 5); // osrs_t[7:5] 

    bme280_read_calibration(bme280_calib);
}

void bme280_read_regs(uint8_t start_reg_address, uint8_t* buffer, uint8_t length){
    uint8_t data[1] = {start_reg_address};
    bme280_ctx.i2c_write(data, sizeof(data));
    bme280_ctx.i2c_read(buffer, length);
}

void bme280_write_reg(uint8_t reg_address, uint8_t value){
    uint8_t data_s[1] = {reg_address};
    bme280_ctx.i2c_write(data_s, sizeof(data_s));

    uint8_t data[2] = {reg_address, value};
    bme280_ctx.i2c_write(data, sizeof(data));
}

void bme280_read_calibration(bme280_calib_data *cal) {
    uint8_t data[32];
    
    read_i2c(BME280_ADDR, 0x88, data, 24);
    read_i2c(BME280_ADDR, 0xE1, data+24, 7);
   
    cal->dig_T1 = data[0] | (data[1] << 8);
    cal->dig_T2 = (int16_t)(data[2] | (data[3] << 8));
    cal->dig_T3 = (int16_t)(data[4] | (data[5] << 8));
    
    cal->dig_P1 = data[6] | (data[7] << 8);
    cal->dig_P2 = (int16_t)(data[8] | (data[9] << 8));
    cal->dig_P3 = (int16_t)(data[10] | (data[11] << 8));
    cal->dig_P4 = (int16_t)(data[12] | (data[13] << 8));
    cal->dig_P5 = (int16_t)(data[14] | (data[15] << 8));
    cal->dig_P6 = (int16_t)(data[16] | (data[17] << 8));
    cal->dig_P7 = (int16_t)(data[18] | (data[19] << 8));
    cal->dig_P8 = (int16_t)(data[20] | (data[21] << 8));
    cal->dig_P9 = (int16_t)(data[22] | (data[23] << 8));
    
    cal->dig_H1 = data[24];
    cal->dig_H2 = (int16_t)(data[25] | (data[26] << 8));
    cal->dig_H3 = data[27];
    cal->dig_H4 = (int16_t)((data[28] << 4) | (data[29] & 0x0F));
    cal->dig_H5 = (int16_t)((data[30] << 4) | (data[29] >> 4));
    cal->dig_H6 = data[31];
}

uint16_t bme280_read_temp_raw()
{
	uint8_t read[2] = {0};
	bme280_read_regs(BME280_REG_temp_msb, read, sizeof(read));
	uint16_t value = ((uint16_t)read[0] << 8) | ((uint16_t)read[1]);
	return value;
}

uint16_t bme280_read_pres_raw()
{
    uint8_t read[2] = {0};
	bme280_read_regs(BME280_RED_press_msb, read, sizeof(read));
	uint16_t value = ((uint16_t)read[0] << 8) | ((uint16_t)read[1]);
	return value;
}

uint16_t bme280_read_hum_raw()
{
    uint8_t read[2] = {0};
	bme280_read_regs(BME280_RED_hum_msb, read, sizeof(read));
	uint16_t value = ((uint16_t)read[0] << 8) | ((uint16_t)read[1]);
	return value;
}

//---------- Температура, давление, влажность ----------

#define BME280_S32_t int32_t
#define BME280_U32_t uint32_t
#define BME280_S64_t int64_t

BME280_S32_t t_fine; 
BME280_S32_t BME280_compensate_T_int32(BME280_S32_t adc_T, bme280_calib_data *bme280_calib) 
{ 
    BME280_S32_t var1, var2, T; 
    var1 = ((((adc_T>>3) - ((BME280_S32_t)bme280_calib->dig_T1<<1))) * ((BME280_S32_t)bme280_calib->dig_T2)) >> 11; 
    var2 = (((((adc_T>>4) - ((BME280_S32_t)bme280_calib->dig_T1)) * ((adc_T>>4) - ((BME280_S32_t)bme280_calib->dig_T1))) >> 12) * ((BME280_S32_t)bme280_calib->dig_T3)) >> 14; 
    t_fine = var1 + var2; 
    T = (t_fine * 5 + 128) >> 8; 
    return T; 
}

BME280_U32_t BME280_compensate_P_int64(BME280_S32_t adc_P, bme280_calib_data *bme280_calib) 
{ 
    BME280_S64_t var1, var2, p; 
    var1 = ((BME280_S64_t)t_fine) - 128000; 
    var2 = var1 * var1 * (BME280_S64_t)bme280_calib->dig_P6; 
    var2 = var2 + ((var1 * (BME280_S64_t)bme280_calib->dig_P5) << 17); 
    var2 = var2 + (((BME280_S64_t)bme280_calib->dig_P4) << 35); 
    var1 = ((var1 * var1 * (BME280_S64_t)bme280_calib->dig_P3) >> 8) + ((var1 * (BME280_S64_t)bme280_calib->dig_P2) << 12); 
    var1 = (((((BME280_S64_t)1) << 47) + var1)) * ((BME280_S64_t)bme280_calib->dig_P1) >> 33; 
    
    if (var1 == 0) { 
        return 0; // avoid exception caused by division by zero 
    } 
    
    p = 1048576 - adc_P; 
    p = (((p << 31) - var2) * 3125) / var1; 
    var1 = (((BME280_S64_t)bme280_calib->dig_P9) * (p >> 13) * (p >> 13)) >> 25; 
    var2 = (((BME280_S64_t)bme280_calib->dig_P8) * p) >> 19; 
    p = ((p + var1 + var2) >> 8) + (((BME280_S64_t)bme280_calib->dig_P7) << 4); 
    
    return (BME280_U32_t)p; 
}

BME280_U32_t bme280_compensate_H_int32(BME280_S32_t adc_H, bme280_calib_data *bme280_calib) 
{ 
    BME280_S32_t v_x1_u32r; 
    v_x1_u32r = (t_fine - ((BME280_S32_t)76800));  
    v_x1_u32r = (((((adc_H << 14) - (((BME280_S32_t)bme280_calib->dig_H4) << 20) - (((BME280_S32_t)bme280_calib->dig_H5) * v_x1_u32r)) + ((BME280_S32_t)16384)) >> 15) * (((((((v_x1_u32r * ((BME280_S32_t)bme280_calib->dig_H6)) >> 10) * (((v_x1_u32r * ((BME280_S32_t)bme280_calib->dig_H3)) >> 11) + ((BME280_S32_t)32768))) >> 10) + ((BME280_S32_t)2097152)) * ((BME280_S32_t)bme280_calib->dig_H2) + 8192) >> 14); 
    v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) * ((BME280_S32_t)bme280_calib->dig_H1)) >> 4)); 
    v_x1_u32r = (v_x1_u32r < 0 ? 0 : v_x1_u32r); 
    v_x1_u32r = (v_x1_u32r > 419430400 ? 419430400 : v_x1_u32r); 
    return (BME280_U32_t)(v_x1_u32r >> 12); 
}

