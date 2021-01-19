#include "globals.h"
#include "helpers.h"
#include "logging.h"
#include "interfaces/i2c.h"
#include <avr/io.h>
#include <util/delay.h>
#include "drivers/bme280.h"
#include "controller/actor.h"

struct state;
typedef void state_fn(struct state*);

struct state
{
  state_fn* next;
  int pass_data;
};

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
  state->next = wait_for_touch;
}


int main(void)
{
  // run setup/initialization functions
  uart_init();
  I2CInit();
  bme280_init();
  setup_heating_element();
  OCR2B = 0x00;


  // Initialize state machine with initial state "wait for touch"
  // (waiting for user input after init)
  //struct state state = { wait_for_touch, 0 };

  int cntr =0;

  while (1) {
    deaktivate_heating_pwm();
    _delay_ms(1000);
    aktivate_heating_pwm();
    //_delay_ms(1000);
    cntr+=50;
    if(cntr>255)
    {
      cntr=0;
    }
    update_heating_dutycycle(cntr);
    _delay_ms(1000);
    
  }

  // can never be reached
  return 0;
}