#include "i2c.h"
#include "../globals.h"
#include "../helpers.h"
#include "../logging.h"
#include <avr/io.h>
#include <util/twi.h>

void i2c_init(void)
{
    //set SCL to 400kHZ
    //TWSR – TWI Status Register (p.200)
    //Leave Prescaler at 1 (TWPS0..1 set to 0)
    //Bit Rate Generator Unit - (p.180)
    //Arduino uses 12MHz CPU Clock, Touch Sensors handles max 400kHZ SCL speed (maybe slower speed is better?)
    //from Calculation 400kHZ= 12MHz/(16+2*(TWBR)*Prescaler) --> TWBR = 7
    TWBR = SET_TWBR;

    //enable TWI, TWEN must be written to one to enable the 2-wire serial interface (p. 188)
    TWCR = bitValue(TWEN);
}

void  i2c_start(void)
{
    //send start condition (p.188)
    TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
    
    while(!(TWCR & (1<<TWINT)));
}

void i2c_start_address(uint8_t addr)
{
    uint8_t twst;
    //send start condition (p.188)
    TWCR = 0;
    TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
    while(!(TWCR & (1<<TWINT)));

    twst = i2c_get_status();
    LOG_DEBUG(I2C_DEEP,"Start Status: %x", twst);
    //a start condition has been transmitted
    if ((twst!=TW_START) && (twst!=TW_REP_START))
    {
        LOG_DEBUG(I2C,"Start failed with Code: %x", twst);
        return;
    }

    TWDR = addr;
    TWCR = (1<<TWINT)|(1<<TWEN);
    while (!(TWCR & (1<<TWINT)));

    twst = i2c_get_status();
    LOG_DEBUG(I2C_DEEP,"Start Addr Status: %x", twst);
    if ((twst != TW_MT_SLA_ACK) && (twst!= TW_MR_SLA_ACK))
    {
        LOG_DEBUG(I2C, "%x",twst);
        return;
    }

    return;
}

void i2c_restart_address(uint8_t addr)
{
    uint8_t twst;
    //send start condition (p.188)
    TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
    while(!(TWCR & (1<<TWINT)));

    twst = i2c_get_status();
    LOG_DEBUG(I2C_DEEP,"Start Status: %x", twst);
    //A start condition has been transmitted
    if ((twst!=TW_START) && (twst!=TW_REP_START))
    {
        LOG_DEBUG(I2C,"Start failed with Code: %x", twst);
        return;
    }

    TWDR = addr;
    TWCR = (1<<TWINT)|(1<<TWEN);
    while (!(TWCR & (1<<TWINT)));

    twst = i2c_get_status();
    LOG_DEBUG(I2C_DEEP,"Start Addr Status: %x", twst);
    if ((twst != TW_MT_SLA_ACK) && (twst!= TW_MR_SLA_ACK))
    {
        LOG_DEBUG(I2C, "%x",twst);
        return;
    }

    return;
}

//Send Stop Condition
void i2c_stop(void)
{
    TWCR = (1<<TWINT)|(1<<TWSTO)|(1<<TWEN);
}

void i2c_write(uint8_t u8data)
{
    uint8_t twst;
    TWDR = u8data;
    TWCR = (1<<TWINT)|(1<<TWEN);

    while (!(TWCR & (1<<TWINT)));

    twst = i2c_get_status();

    LOG_DEBUG(I2C_DEEP,"Write Status: %x", twst);
    if(twst != TW_MT_DATA_ACK){

        LOG_DEBUG(I2C,"%x", twst);
        return;
    } 

    return;
}

void i2c_write_data(uint8_t addr, uint8_t data)
{
    i2c_write(addr);
    i2c_write(data);
}


uint8_t i2c_read_ack(void)
{
    uint8_t twst;
    TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWEA);
    while (!(TWCR & (1<<TWINT)));

    twst = i2c_get_status();
    LOG_DEBUG(I2C_DEEP,"ReadACK Status: %x", twst);

    return TWDR;
}

uint8_t i2c_read_no_ack(void)
{
    uint8_t twst;
    TWCR = (1<<TWINT)|(1<<TWEN);
    while (!(TWCR & (1<<TWINT)));

    twst = i2c_get_status();
    LOG_DEBUG(I2C_DEEP,"ReadACK Status: %x", twst);

    return TWDR;
}

//read the current status from status register
uint8_t i2c_get_status(void)
{
    uint8_t status;
    //mask status,
    status = TW_STATUS & 0xF8;
    return status;
}

