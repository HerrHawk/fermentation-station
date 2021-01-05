#define BME280_ADDRESS (0x76)

#include <avr/io.h>

enum {
    BME280_REG_ID = 0xD0,
    BME280_REG_RESET = 0xE0,
    BME280_REG_CTRL_HUM = 0xF2, //(BME280 p.28)
    BME280_REG_CTRL_MEAS = 0xF4,
    BME280_REG_CONFIG = 0xF5,//(p.30)
    BME280_REG_DATA_PRESS = 0XF7,//(p.31)
    BME280_REG_DATA_TEMP = 0xFA,
    BME280_REG_DATA_HUM = 0xFD, 
    //Trimming parameter register addresses
    BME280_REG_CALIB_T1 = 0x88,
    BME280_REG_CALIB_T2 = 0x8A,
    BME280_REG_CALIB_T3 = 0x8C,

    BME280_REG_CALIB_P1 = 0x8E,
    BME280_REG_CALIB_P2 = 0x90,
    BME280_REG_CALIB_P3 = 0x92,
    BME280_REG_CALIB_P4 = 0x94,
    BME280_REG_CALIB_P5 = 0x96,
    BME280_REG_CALIB_P6 = 0x98,
    BME280_REG_CALIB_P7 = 0x9A,
    BME280_REG_CALIB_P8 = 0x9C,
    BME280_REG_CALIB_P9 = 0x9E,

    BME280_REG_CALIB_H1 = 0xA1,
    BME280_REG_CALIB_H2 = 0xE1,
    BME280_REG_CALIB_H3 = 0xE3,
    BME280_REG_CALIB_H4 = 0xE4,
    BME280_REG_CALIB_H5 = 0xE5,
    BME280_REG_CALIB_H6 = 0xE7
 };

 typedef struct 
 {
    uint16_t    dig_T1;
    int16_t     dig_T2;
    int16_t     dig_T3;

    uint16_t    dig_P1;
    int16_t     dig_P2;
    int16_t     dig_P3;
    int16_t     dig_P4;
    int16_t     dig_P5;
    int16_t     dig_P6;
    int16_t     dig_P7;
    int16_t     dig_P8;
    int16_t     dig_P9;

    uint8_t     dig_H1;
    int16_t     dig_H2;
    uint8_t     dig_H3;
    int16_t     dig_H4;
    int16_t     dig_H5;
    uint8_t     dig_H6;
 } trimming_params;


void bme280_init();
uint8_t bme280_read_byte(uint8_t addr);
uint16_t bme280_read_2bytes(uint8_t addr);
uint32_t bme280_read_3bytes(uint8_t addr);
uint16_t bme280_read_2bytes_remapped(uint8_t addr);

void bme280_read_trimming_register();

int32_t bme280_read_temp();

uint32_t bme280_read_hum();