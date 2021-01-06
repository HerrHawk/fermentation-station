#include "mpr121.h"
#include "../interfaces/i2c.h"
#include "../logging.h"
#include "util/delay.h"

 void mpr121_init()
 {
    LOG_DEBUG(I2C, "Info: MPR121 start init.");
    //Softreset
    I2CStartAddress(MPR121_ADDRESS<<1);
    I2CWrite(MPR121_REG_SOFTRESET);
    I2CWrite(0x63);
    I2CStop();
    _delay_ms(20);
    
    I2CStartAddress(MPR121_ADDRESS<<1);
    I2CWrite(MPR121_REG_ECR);
    I2CWrite(0x00);
    I2CStop();
    _delay_ms(20);

   uint8_t check = mpr121_read_byte(0x5D);
    if(check != 0x24)
    { 
      LOG_DEBUG(I2C, "%x",check);
      LOG_DEBUG(I2C, "ERROR: MPR121 not found!");
     // return 1;
    }



    I2CStartAddress(MPR121_ADDRESS<<1);
    I2CWrite(MPR121_MHDR);
    I2CWrite(0x01);

    I2CWrite(MPR121_NHDR);
    I2CWrite(0x01);

    I2CWrite(MPR121_NCLR);
    I2CWrite(0x0E);

    I2CWrite(MPR121_FDLR);
    I2CWrite(0x00);

    I2CWrite(MPR121_MHDF);
    I2CWrite(0x01);

    I2CWrite(MPR121_NHDF);
    I2CWrite(0x05);

    I2CWrite(MPR121_NCLF);
    I2CWrite(0x01);

    I2CWrite(MPR121_FDLF);
    I2CWrite(0x00);

    I2CWrite(MPR121_NHDT);
    I2CWrite(0x00);

    I2CWrite(MPR121_NCLT);
    I2CWrite(0x00);

    I2CWrite(MPR121_FDLT);
    I2CWrite(0x00);

   I2CWrite(0x5E);
   I2CWrite(0x8F);
   I2CStop();

   check = mpr121_read_byte(0x5E);
    if(check != 0x24)
    { 
      LOG_DEBUG(I2C, "%x",check);
      LOG_DEBUG(I2C, "ERROR: MPR121 not found!");
     // return 1;
    }
    
    LOG_DEBUG(I2C, "Info: MPR121 connected.");

    
 }

 uint8_t mpr121_read_byte(uint8_t addr)
 {
    uint8_t ret_val;
    I2CStartAddress(MPR121_ADDRESS<<1);
    I2CWrite(addr);
    //I2CStop();

    I2CStartAddress(MPR121_ADDRESS<<1|0x01);
    ret_val = I2CReadACK();

    LOG_DEBUG(I2C, "Return %x",ret_val);
    I2CStop();

    return ret_val;
 }