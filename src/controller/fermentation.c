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
  LOG_DEBUG(TEMPERATURE, "Current Temperature is %d ", current_temp);

  if (current_temp <= min_temp) {
    // turn on heating system
    LOG_DEBUG(TEMPERATURE, "Temp is below hystherese threshold. Turning on heating system");
  } else if (current_temp >= max_temp) {
    // turn off heating system
    LOG_DEBUG(TEMPERATURE, "Temp is above hystherese threshold. Turning off heating system");
  }
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