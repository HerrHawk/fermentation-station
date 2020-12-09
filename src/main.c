#include "helpers.h"
#include "logging.h"
#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>

int main(void)
{
  // run once
  uart_init();
  stdout = &uart_output;
  stdin = &uart_input;
  // set which pins to use as output in the Data Direction Register
  // DDRB = (1 << PB5);

  // // switch on LEDs
  // while (1) {
  //   setBit(PORTB, PB5);
  //   _delay_ms(200);
  //   clearBit(PORTB, PB5);
  //   _delay_ms(200);
  //   toggleBit(PORTB, PB5);
  //   _delay_ms(200);
  //   toggleBit(PORTB, PB5);
  //   _delay_ms(1000);
  // }

  // can never be reached
  return 0;
}
