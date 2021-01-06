#include "globals.h"
#include "helpers.h"
#include "logging.h"
#include "interfaces/i2c.h"
#include <avr/io.h>
#include <util/delay.h>
#include "drivers/bme280.h"
#include "drivers/mpr121.h"

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

// Dummy method for occurring errors
void error_function()
{
  LOG_DEBUG(DEFAULT, "error occurred, writing to log");
  _delay_ms(2500);
}

int main(void)
{
  // run setup/initialization functions

  uart_init();
  I2CInit();
  _delay_ms(100);
  mpr121_init();
  bme280_init();
  

  // Initialize state machine with initial state "wait for touch"
  // (waiting for user input after init)
  struct state state = { wait_for_touch, 0 };

  // while (1) {
  //   //
  //   state.next(&state);
  //   setBit(PORTB, PB5);
  //   _delay_ms(200);
  //   clearBit(PORTB, PB5);
  //   _delay_ms(200);
  //   toggleBit(PORTB, PB5);
  //   _delay_ms(200);
  //   toggleBit(PORTB, PB5);
  //   LOG_DEBUG(DEFAULT, "Main Test Routine");
  //   _delay_ms(1000);
  //   LOG_DEBUG(DEFAULT, "Start BME280 Test");
  //   int32_t temp =  bme280_read_temp();
  //   LOG_DEBUG(DEFAULT, "Temperature: %ld", temp);
  //   uint32_t hum = bme280_read_hum();

  //   hum/= 1024;
  //   LOG_DEBUG(DEFAULT, "Humidity: %ul", hum);
  //   LOG_DEBUG(DEFAULT, "End BME280 Test");

  //   LOG_DEBUG(DEFAULT, "touch: %x", mpr121_read_byte(0x00));

  //   _delay_ms(100);

  //}

  // can never be reached
  return 0;
}