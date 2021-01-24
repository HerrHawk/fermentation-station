#include "controller/fermentation.h"
#include "drivers/bme280.h"
#include "drivers/mpr121.h"
#include "globals.h"
#include "helpers.h"
#include "logging.h"
#include "interfaces/i2c.h"
#include <avr/io.h>
#include <util/delay.h>
#include "timer.h"

struct state;
typedef void state_fn(struct state*);

struct state
{
  state_fn* next;
  int pass_data;
};

struct recipe recipes[] = {
  { "Lactobacillales",
    2500,
    -1,
    0 }, // Lactofermentierung, 25° (28° optimal), no humidity (-1 means perma off)
  { "SCOBY",
    2500,
    -1,
    0 }, // Symbiotic Culture of Bacteria and Yeast, 25° (28° optimal), no hum (-1)
  { "Aspergillus Orycae", 3000, 72000, 7000 },// Koji, 30°, 72% humidity
  { "Black Garlic", 6000,  72000, 7000 }
};

int main(void)
{
  // run setup/initialization functions
  uart_init();
  I2CInit();
  _delay_ms(100);
  //mpr121_init();
  bme280_init();
  setup_heating_element();
  setup_timer_s1();

  activate_heating_pwm();

  while (1) {

    if(s1_triggered)
    {
      check_temp(&recipes[3]);
      s1_triggered = 0;
    }     
  }
  return 0;
}
