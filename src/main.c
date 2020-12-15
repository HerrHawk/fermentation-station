#include "helpers.h"
#include "logging.h"
#include <avr/io.h>
#include <util/delay.h>

int main(void)
{
  // run setup functions
  uart_init();

  while (1) {
    setBit(PORTB, PB5);
    _delay_ms(200);
    clearBit(PORTB, PB5);
    _delay_ms(200);
    toggleBit(PORTB, PB5);
    _delay_ms(200);
    toggleBit(PORTB, PB5);
    LOG_DEBUG(DEFAULT, "hello there chap");
    LOG_DEBUG(TEMPERATURE, "hello there temp");
    _delay_ms(1000);
  }

  // can never be reached
  return 0;
}