#include "bme280.h"
#include "../interfaces/i2c.h"
#include "../logging.h"
#include "util/delay.h"

trimming_params t_p;
int32_t t_fine;

void bme280_init()
{
    //Check if Sensor exists, should return 0x60 (ID of Sensor)
    uint8_t id = bme280_read_byte(BME280_REG_ID);
    if(id != 0x60)
    {
        LOG_DEBUG(TEMPERATURE, "ERROR: BME280 not found!");

        return 1;
    }

    LOG_DEBUG(TEMPERATURE, "Info: BME280 connected.");

    I2CStartAddress(BME280_ADDRESS<<1);
    //reset with complete power on reset procedure (BME280 p.27)
    I2CWrite(BME280_REG_RESET);
    I2CWrite(0xB6);
    I2CStop();
    //wait for finish
    //startup time is 2 ms .. just to be safe
    _delay_ms(20);

    //write config 
    I2CStartAddress(BME280_ADDRESS<<1);
    //config for humidity
    I2CWrite(BME280_REG_CTRL_HUM);
    //set humidity oversempling to 8
    I2CWrite(0x04);
    //config the rate, filter and interface
    I2CWrite(BME280_REG_CONFIG);
    //set standby to 250 ms(0x03), filter to 8(0x03), spi off (0x00)
    I2CWrite((0x03<<5)|(0x03<<2)|(0x00));

    //config pressure, temperature and sensor mode
    I2CWrite(BME280_REG_CTRL_MEAS);
    //temp oversampling to 8, pressure oversampling to 8, sensor mode to normal
    I2CWrite((0x04<<5)|(0x04<<2)|(0x03));

    I2CStop();

    bme280_read_trimming_register();

    LOG_DEBUG(I2C, "Read trimming: ");
    LOG_DEBUG(I2C, "%x", t_p.dig_T1);
    LOG_DEBUG(I2C, "%x", t_p.dig_P1);    
    LOG_DEBUG(I2C, "%x", t_p.dig_H1);
    LOG_DEBUG(I2C, "%x", t_p.dig_T2);  
}

uint8_t bme280_read_byte(uint8_t addr)
{
    uint8_t ret_val;
    I2CStartAddress(BME280_ADDRESS<<1);
    I2CWrite(addr);
    I2CStop();

    I2CStartAddress(BME280_ADDRESS<<1|0x01);
    ret_val = I2CReadNoACK();
    I2CStop();

    return ret_val;
}

uint16_t bme280_read_2bytes(uint8_t addr)
{
    uint16_t ret_val;
    
    //"aktivate" read-register
    I2CStartAddress(BME280_ADDRESS<<1);
    I2CWrite(addr);
    I2CStop();

    //Start read with address & read flag
    I2CStartAddress(BME280_ADDRESS<<1|0x01);
    //Read 1st Byte, send Ack & shift
    ret_val = I2CReadACK();
    ret_val <<= 8;
    //Read 2nd Byte, set in return value
    ret_val |= I2CReadNoACK();
    I2CStop();
    
    return ret_val;
}

uint16_t bme280_read_2bytes_remapped(uint8_t addr)
{
    uint16_t ret_val;
    ret_val = bme280_read_2bytes(addr);
    return (ret_val << 8) | (ret_val >> 8 );
}


uint32_t bme280_read_3bytes(uint8_t addr)
{
    uint32_t ret_val;

    //"aktivate" read-register
    I2CStartAddress(BME280_ADDRESS<<1);
    I2CWrite(addr);
    I2CStop();

    //Start read with address & read flag
    I2CStartAddress(BME280_ADDRESS<<1|0x01);
    //Read 1st Byte, send Ack & shift
    ret_val = I2CReadACK();
    ret_val <<= 8;
    ret_val |= I2CReadACK();
    ret_val <<=8;
    ret_val |=  I2CReadNoACK();
    I2CStop();

    return ret_val;
}

void bme280_read_trimming_register()
{
    t_p.dig_T1 = bme280_read_2bytes_remapped(BME280_REG_CALIB_T1);
    t_p.dig_T2 = (int16_t)bme280_read_2bytes_remapped(BME280_REG_CALIB_T2);  
    t_p.dig_T3 = (int16_t)bme280_read_2bytes_remapped(BME280_REG_CALIB_T3);

    t_p.dig_P1 = bme280_read_2bytes_remapped(BME280_REG_CALIB_P1);
    t_p.dig_P2 = (int16_t)bme280_read_2bytes_remapped(BME280_REG_CALIB_P2);
    t_p.dig_P3 = (int16_t)bme280_read_2bytes_remapped(BME280_REG_CALIB_P3);
    t_p.dig_P4 = (int16_t)bme280_read_2bytes_remapped(BME280_REG_CALIB_P4);
    t_p.dig_P5 = (int16_t)bme280_read_2bytes_remapped(BME280_REG_CALIB_P5);
    t_p.dig_P6 = (int16_t)bme280_read_2bytes_remapped(BME280_REG_CALIB_P6);
    t_p.dig_P7 = (int16_t)bme280_read_2bytes_remapped(BME280_REG_CALIB_P7);
    t_p.dig_P8 = (int16_t)bme280_read_2bytes_remapped(BME280_REG_CALIB_P8);
    t_p.dig_P9 = (int16_t)bme280_read_2bytes_remapped(BME280_REG_CALIB_P9);
    
    t_p.dig_H1 = bme280_read_byte(BME280_REG_CALIB_H1);
    t_p.dig_H2 = (int16_t)bme280_read_2bytes_remapped(BME280_REG_CALIB_H2);
    t_p.dig_H3 = bme280_read_byte(BME280_REG_CALIB_H3);
    t_p.dig_H4 = (bme280_read_byte(BME280_REG_CALIB_H4)<<4)|(bme280_read_byte(BME280_REG_CALIB_H4+1)&0xF);
    t_p.dig_H5 = (bme280_read_byte(BME280_REG_CALIB_H5+1)<<4)|(bme280_read_byte(BME280_REG_CALIB_H5)>>4);
    t_p.dig_H6 = bme280_read_byte(BME280_REG_CALIB_H6);
}

int32_t bme280_read_temp()
{
    int32_t tempraw;
    tempraw = (int32_t)bme280_read_3bytes(BME280_REG_DATA_TEMP);
    tempraw >>=4;
    //compensation formular for temperature. From BME280 datasheet (p.25)
    int32_t var1,var2, T;
    
    var1 = ((((tempraw>>3)-((int32_t)t_p.dig_T1<<1))) * ((int32_t)t_p.dig_T2)) >> 11;
    var2 = (((((tempraw>>4)-((int32_t)t_p.dig_T1))*((tempraw>>4)-((int32_t)t_p.dig_T1)))>>12) *
    ((int32_t)t_p.dig_T3)) >> 14;

    t_fine = var1+var2;
    T = (t_fine*5+128)>>8;
    return T;
}

uint32_t bme280_read_hum()
{
    int32_t humraw = (int32_t)bme280_read_2bytes(BME280_REG_DATA_HUM);

    uint32_t var1;

    var1 = (t_fine- ((int32_t)76800));
    var1 = (((((humraw<<14)-(((int32_t)t_p.dig_H4)<<20)-(((int32_t)t_p.dig_H5)*
        var1))+((int32_t)16284))>>15)*(((((((var1*
        ((int32_t)t_p.dig_H6))>>10)*(((var1*((int32_t)t_p.dig_H3))>>11)+
        ((int32_t)32768)))>>10)+((int32_t)2097152))*((int32_t)t_p.dig_H2)+
        8192)>>14));
    
    var1 = (var1 - (((((var1>>15)*(var1>>15))>>7)*
    ((int32_t)t_p.dig_H1))>>4));
    var1 = (var1 < 0 ? 0 : var1);
    var1 = (var1 > 419430400? 419430400 : var1);

    return (uint32_t)(var1>>12);
}