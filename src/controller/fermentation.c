#include "fermentation.h"
#include "../drivers/bme280.h"
#include "../globals.h"
#include "../helpers.h"
#include "../logging.h"
#include <avr/io.h>
#include <util/delay.h>

void check_temp(struct recipe* current_recipe)
{
  int32_t min_temp = current_recipe->desired_temp - current_recipe->temp_hyst;
  int32_t max_temp = current_recipe->desired_temp + current_recipe->temp_hyst;

  int32_t current_temp = bme280_read_temp();
  LOG_DEBUG(TEMPERATURE, "Temp %d ", current_temp);

  int32_t control_output = pid_calculate(current_recipe);
  LOG_DEBUG(DEFAULT, "Pid %d", control_output);
 
  //int8_t pwm ;

  //LOG_DEBUG(DEFAULT, "PWM Map: %d", pwm);
  //LOG_DEBUG(DEFAULT, "==========================");

  if(control_output>255){
    OCR2B = 0xFF;
  }else if(control_output<0){
    OCR2B = 0x00;
  }
  else
  {
    OCR2B = control_output;
  }
  

  // if (current_temp <= min_temp) {
  //   // turn on heating system
  //   LOG_DEBUG(TEMPERATURE, "Temp is below hystherese threshold. Turning on heating system");
  // } else if (current_temp >= max_temp) {
  //   // turn off heating system
  //   LOG_DEBUG(TEMPERATURE, "Temp is above hystherese threshold. Turning off heating system");
  // }
}

void check_hum(struct recipe* current_recipe)
{
  // Humidity does not affect some recipes ->
  // if the value -1 is set in the recipe humidity can be ignored
  if (current_recipe->desired_hum == -1) {
    return;
  }
  uint32_t min_hum = current_recipe->desired_hum - current_recipe->hum_hyst;
  uint32_t max_hum = current_recipe->desired_hum + current_recipe->hum_hyst;

  uint32_t current_hum = bme280_read_hum();
  LOG_DEBUG(HUMIDITY, "Current Humidity is %u ", current_hum);

  if (current_hum <= min_hum) {
    // turn on heating system
    LOG_DEBUG(HUMIDITY, "Hum is below hystherese threshold. Turning on humidifer");
  } else if (current_hum >= max_hum) {
    // turn off heating system
    LOG_DEBUG(HUMIDITY, "Hum is above hystherese threshold. Turning off humidifier");
  }
}

// TEST - also for hum?
// This test assumes the scenario that the temp will be checked every 1s
// Therefore its assumed that the dervivative ...

// for derivative calc of pid
int32_t prev_temp = -1;
int32_t prev_error = -1;
int32_t prev_time = -1;
int32_t integral = 0;

int32_t pid_calculate(struct recipe* rec)
{

  int32_t control_output = 0;

  int32_t current_time = 0;                  // TODO: TIME
  int32_t desired_temp = rec->desired_temp;  // eq. setpoint
  int32_t current_temp = bme280_read_temp(); // eq. input
  int32_t error = desired_temp - current_temp;

  if (prev_temp == -1) {
    prev_temp = current_temp;
  }

  if (prev_time == -1) {
    prev_time = current_time;
  }

  if (prev_error == -1) {
    prev_error = error;
  }

  // int32_t time_change = current_time - prev_time;
  int32_t time_change = 1; // <- fixed time intervals of 1 second?

  // TODO: Get meaningful values
  // PID -> GainP + GainI + GainD
  double kP = 3;
  double kI = 0.1;
  double kD = 1.0;

  // https://forum.arduino.cc/index.php?topic=430374.0
  int32_t gainP = kP * error;
  if(-10<error && error<10){
      integral += error * time_change;
  }
  int32_t gainI = kI * integral;
  int32_t gainD = kD * ((error - prev_error) / time_change);

  //LOG_DEBUG(DEFAULT, "gainP %ld", gainP);
  //(DEFAULT, "integral %ld", integral);
  //(DEFAULT, "gainI %ld", gainI);

  control_output = gainP + gainI + gainD;

  prev_temp = current_temp;
  prev_error = error;
  // prev_time = 0; // TODO: TIME

  return control_output;
}
