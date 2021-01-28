#include "mpr121.h"
#include "../interfaces/i2c.h"
#include "../logging.h"
#include "util/delay.h"

 void mpr121_init()
 {
    LOG_DEBUG(I2C, "Info: MPR121 start init.");
    _delay_ms(1000);
    
    //Softreset
    I2CStartAddress(MPR121_ADDRESS<<1);
    I2CWrite(MPR121_REG_SOFTRESET);
    I2CWrite(0x63);
    I2CStop();
    _delay_ms(20);

   //Check for default config 
   uint8_t check = mpr121_read_byte(0x5D);
   if(check != 0x24)
   { 
      LOG_DEBUG(I2C, "%x",check);
      LOG_DEBUG(I2C, "ERROR: MPR121 not found!");
      return;
   }
    
   //Set MPR to Off_
    I2CStartAddress(MPR121_ADDRESS<<1);
    I2CWrite(MPR121_REG_ECR);
    I2CWrite(0x00);
    I2CStop();
    _delay_ms(20);

   //Recommended Settings
   write_data(MPR121_MHDR, 0X01);
   write_data(MPR121_NHDR, 0X01);
   write_data(MPR121_NCLR, 0X00);
   write_data(MPR121_FDLR, 0X00);

   write_data(MPR121_MHDF,0x01);
   write_data(MPR121_NHDF,0x01);
   write_data(MPR121_NCLF,0xFF);
   write_data(MPR121_FDLF,0x00);

   write_data(MPR121_REG_DEBOUNCE,0x02);
   write_data(MPR121_REG_CONFIG2,0x04);


   //set_e_thresholds();
   //Autoconfig
   //write_data(MPR121_REG_AUTOCONFIG_0,0x0B);
   //write_data(MPR121_REG_USL,200);
   //write_data(MPR121_REG_LSL,130);
  // write_data(MPR121_REG_TL,180);

   write_data(MPR121_REG_ECR,0x0C);
   I2CStop();

    
    LOG_DEBUG(I2C, "Info: MPR121 Setup Complete.");
 }


void write_data(uint8_t addr, uint8_t data)
{
   I2CStartAddress(MPR121_ADDRESS<<1);
   I2CWrite(addr);
   I2CWrite(data);
   I2CStop();
}

 uint8_t mpr121_read_byte(uint8_t addr)
 {
   uint8_t ret_val;
   I2CStartAddress(MPR121_ADDRESS<<1);
   I2CWrite(addr);   
   I2CReStartAddress(MPR121_ADDRESS<<1|0x01);
   ret_val = I2CReadACK();
   I2CStop();
   return ret_val;
 }

 void set_e_thresholds(){
   //I2CStartAddress(MPR121_ADDRESS<<1);
   I2CStartAddress(MPR121_ADDRESS<<1);
   I2CWriteData(E0TTH,TThre);
   I2CWriteData(E0RTH,RThre);

   I2CWriteData(E1TTH,TThre);
   I2CWriteData(E1RTH,RThre);

   I2CWriteData(E2TTH,TThre);
   I2CWriteData(E2RTH,RThre);

   I2CWriteData(E3TTH,TThre);
   I2CWriteData(E3RTH,RThre);

   I2CWriteData(E4TTH,TThre);
   I2CWriteData(E4RTH,RThre);

   I2CWriteData(E5TTH,TThre);
   I2CWriteData(E5RTH,RThre);

   I2CWriteData(E6TTH,TThre);
   I2CWriteData(E6RTH,RThre);
   
   I2CWriteData(E7TTH,TThre);
   I2CWriteData(E7RTH,RThre);

   I2CWriteData(E8TTH,TThre);
   I2CWriteData(E8RTH,RThre);

   I2CWriteData(E9TTH,TThre);
   I2CWriteData(E9RTH,RThre);

   I2CWriteData(E10TTH,TThre);
   I2CWriteData(E10RTH,RThre);

   I2CWriteData(E11TTH,TThre);
   I2CWriteData(E11RTH,RThre);

   I2CStop();
    
 }