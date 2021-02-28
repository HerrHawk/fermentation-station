#pragma once
#include <avr/io.h>

#define F_I2C			100000UL// clock i2c
#define PSC_I2C			1		// prescaler i2c
#define SET_TWBR		(12000000UL/F_I2C-16UL)/(PSC_I2C*2UL)

void i2c_init(void);
void i2c_start(void);
void i2c_start_address(uint8_t addr);
void i2c_restart_address(uint8_t addr);
void i2c_stop(void);
void i2c_write(uint8_t u8data);
void i2c_write_data(uint8_t addr, uint8_t data);
uint8_t i2c_read_ack(void);
uint8_t i2c_read_no_ack(void);
uint8_t i2c_get_status(void);