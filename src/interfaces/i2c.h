#pragma once
#include <avr/io.h>

#define F_I2C			100000UL// clock i2c
#define PSC_I2C			1		// prescaler i2c
#define SET_TWBR		(12000000UL/F_I2C-16UL)/(PSC_I2C*2UL)

void I2CInit(void);
void I2CStart(void);
void I2CStartAddress(uint8_t addr);
void I2CStop(void);
void I2CWrite(uint8_t u8data);
uint8_t I2CReadACK(void);
uint8_t I2CReadNoACK(void);
uint8_t I2CGetStatus(void);