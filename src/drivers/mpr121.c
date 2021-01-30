#include "mpr121.h"
#include "../helpers.h"
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
   write_data(MPR121_NCLR, 0X0E);
   write_data(MPR121_FDLR, 0X00);

   write_data(MPR121_MHDF,0x01);
   write_data(MPR121_NHDF,0x05);
   write_data(MPR121_NCLF,0x01);
   write_data(MPR121_FDLF,0x00);

   write_data(MPR121_NHDT, 0x00);
   write_data(MPR121_NCLT, 0x00);
   write_data(MPR121_FDLT, 0x00);


   write_data(MPR121_REG_DEBOUNCE,0x03);
   write_data(MPR121_REG_CONFIG1, 0x10);
   write_data(MPR121_REG_CONFIG2,0x20);


   set_e_thresholds();
   //Autoconfig
   write_data(MPR121_REG_AUTOCONFIG_0,0x0B);
   write_data(MPR121_REG_USL,200);
   write_data(MPR121_REG_LSL,130);
   write_data(MPR121_REG_TL,180);

   write_data(MPR121_REG_ECR,0x8C);
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

   write_data(E0TTH,TThre);
   write_data(E0RTH,RThre);

   write_data(E1TTH,TThre);
   write_data(E1RTH,RThre);

   write_data(E2TTH,TThre);
   write_data(E2RTH,RThre);

   write_data(E3TTH,TThre);
   write_data(E3RTH,RThre);

   write_data(E4TTH,TThre);
   write_data(E4RTH,RThre);

   write_data(E5TTH,TThre);
   write_data(E5RTH,RThre);

   write_data(E6TTH,TThre);
   write_data(E6RTH,RThre);

   write_data(E7TTH,TThre);
   write_data(E7RTH,RThre);

   write_data(E8TTH,TThre);
   write_data(E8RTH,RThre);

   write_data(E9TTH,TThre);
   write_data(E9RTH,RThre);

   write_data(E10TTH,TThre);
   write_data(E10RTH,RThre);

   write_data(E11TTH,TThre);
   write_data(E11RTH,RThre);

 }


uint8_t get_touch(){
   static uint8_t prevtouched=0;
   uint8_t currenttouched = mpr121_read_byte(0x00);
   uint8_t res=0;

   for(int i = 0;i<3;i++){
      if(!(currenttouched & bitValue(i)) && (prevtouched & bitValue(i)))
      {
         res |= bitValue(i);
      }
   }
   


   prevtouched = currenttouched;
   return res;

}