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
    i2c_start_address(MPR121_ADDRESS<<1);
    i2c_write(MPR121_REG_SOFTRESET);
    i2c_write(0x63);
    i2c_stop();
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
    i2c_start_address(MPR121_ADDRESS<<1);
    i2c_write(MPR121_REG_ECR);
    i2c_write(0x00);
    i2c_stop();
    _delay_ms(20);

   //Recommended calibration settings (https://www.nxp.com/docs/en/application-note/AN4600.pdf)
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
   //Autoconfig, calibrates sensor on startup automatically 
   write_data(MPR121_REG_AUTOCONFIG_0,0x0B);
   write_data(MPR121_REG_USL,200);
   write_data(MPR121_REG_LSL,130);
   write_data(MPR121_REG_TL,180);

   //set run mode, "starts" the sensor
   write_data(MPR121_REG_ECR,0x8C);
   i2c_stop();

    
   LOG_DEBUG(I2C, "Info: MPR121 Setup Complete.");
 }


void write_data(uint8_t addr, uint8_t data)
{
   i2c_start_address(MPR121_ADDRESS<<1);
   i2c_write(addr);
   i2c_write(data);
   i2c_stop();
}

 uint8_t mpr121_read_byte(uint8_t addr)
 {
   uint8_t ret_val;
   i2c_start_address(MPR121_ADDRESS<<1);
   i2c_write(addr);   
   i2c_restart_address(MPR121_ADDRESS<<1|0x01);
   ret_val = i2c_read_ack();
   i2c_stop();
   return ret_val;
 }

 void set_e_thresholds(){
   //Recommended calibration settings (https://www.nxp.com/docs/en/application-note/AN4600.pdf)
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

//returns the touchinput from the first three pins 0-2. Rest is ignored
//returns "touched" only on change between previous and current reading.
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