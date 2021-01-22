#include "controller/fermentation.h"
#include "drivers/bme280.h"
#include "drivers/mpr121.h"
#include "globals.h"
#include "helpers.h"
#include "logging.h"
#include "interfaces/i2c.h"
#include <avr/io.h>
#include <util/delay.h>
#include "drivers/bme280.h"
#include "controller/actor.h"
#include "timer.h"

struct state;
typedef void state_fn(struct state*);

struct state
{
  state_fn* next;
  int pass_data;
};

// TODO: dynamic array?
struct recipe recipes[] = {
  { "Lactobacillales",
    2500,
    300,
    -1,
    0 }, // Lactofermentierung, 25° (28° optimal), no humidity (-1 means perma off)
  { "SCOBY",
    2500,
    300,
    -1,
    0 }, // Symbiotic Culture of Bacteria and Yeast, 25° (28° optimal), no hum (-1)
  { "Aspergillus Orycae", 3000, 300, 72000, 7000 } // Koji, 30°, 72% humidity
};

uint8_t recipe_counter;
uint8_t change_context;

// int r = rand() % 20;

/*
 * SW States:
 *  - Waiting for user (touch) input
 *  - In Fermentation process (does it make sense to divide this into further states?)
 *  - Error state recoverable -> log and go back into user input
 *  - Error state non recoverable -> for unknown errors (mostly debug purpose)
 */

state_fn wait_for_touch;
state_fn wait_for_ferm;
state_fn error;
state_fn fatalerror;

// This function definition will be replaced by the controller layer functions, e.g.
// touch_input or sth similar
void wait_for_touch(struct state* state)
{
  LOG_DEBUG(DEFAULT, "wait for touch input");
  _delay_ms(2000);
  LOG_DEBUG(DEFAULT, "user touch input complete");
  state->next = wait_for_ferm;
}

// This function definition will be replaced by the controller layer functions, e.g.
// start_fermentation or sth similar
void wait_for_ferm(struct state* state)
{
  LOG_DEBUG(DEFAULT, "begin fermentation");
  _delay_ms(5000);
  LOG_DEBUG(DEFAULT, "fermentation complete!");
  state->next = main_menu;
}


int main(void)
{
  // run setup/initialization functions

  uart_init();
  I2CInit();
  _delay_ms(100);
  mpr121_init();
  bme280_init();
  setup_heating_element();
  setup_timer_s1();
  //sei();
  OCR2B = 0x00;


  // Initialize state machine with initial state "wait for touch"
  // (waiting for user input after init)
  //struct state state = { wait_for_touch, 0 };

  int cntr =0;
  int8_t flip = 1;
  //deaktivate_heating_pwm();

  while (1) {

    if(s1_triggered)
    {
      //LOG_DEBUG(DEFAULT, "Cool!");
      if(flip)
      {
        LOG_DEBUG(DEFAULT, "On!");
        aktivate_heating_pwm();
        cntr+=50;
        if(cntr>255)
        {
          cntr=0;
        }
        update_heating_dutycycle(cntr);
        flip = 0;
      }
      else
      {
        LOG_DEBUG(DEFAULT, "Off!");
        deaktivate_heating_pwm();
        flip = 1;
      }

      s1_triggered = 0;
    }     
  }

  // can never be reached
  return 0;
}
