#include "bme280.h"
#include "../interfaces/i2c.h"
#include "../logging.h"
#include "util/delay.h"


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

uint8_t bme280_read_2bytes(uint8_t addr)
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