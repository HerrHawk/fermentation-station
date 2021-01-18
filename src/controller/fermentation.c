#include "../drivers/bme280.h"
#include "../globals.h"
#include "../helpers.h"
#include "../logging.h"
#include <avr/io.h>
#include <util/delay.h>

// const recipe[] predefined_recipes =
// const test = struct recipe = { "test", 1337, 42 };

// TODO: Proper comments 0->react with state change 1->below threshold
uint8_t check_temp(struct recipe* current_recipe)
{
  uint32_t current_temp = bme280_read_temp();

  if (current_temp <= current_recipe.desired_temp) {
    LOG_DEBUG(TEMPERATURE, "Temp is below threshold");
    return 1;
  } else {
    LOG_DEBUG(TEMPERATURE, "Temp is above threshold");
    LOG_DEBUG(TEMPERATURE, "Deactivating heating system");
    return 0;
  }
}

// TODO: Proper comments
// 0 -> react with state change
// 1 -> below threshold
uint8_t check_hum(struct recipe* current_recipe)
{
  uint32_t current_hum = bme280_read_hum();

  if (current_temp <= current_recipe.desired_hum) {
    LOG_DEBUG(HUMIDITY, "Temp is below threshold");
    return 1;
  } else {
    LOG_DEBUG(HUMIDITY, "Temp is above threshold");
    LOG_DEBUG(HUMIDITY, "Deactivating heating system");
    return 0;
  }
}