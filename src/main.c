#include "helpers.h"
#include "logging.h"
#include "interfaces/i2c.h"
#include <avr/io.h>
#include <util/delay.h>
#include "drivers/bme280.h"





int main(void)
{
  // run setup functions
  uart_init();
  I2CInit();

  while (1) {
    setBit(PORTB, PB5);
    _delay_ms(200);
    clearBit(PORTB, PB5);
    _delay_ms(200);
    toggleBit(PORTB, PB5);
    _delay_ms(200);
    toggleBit(PORTB, PB5);
    LOG_DEBUG(DEFAULT, "Main Test Routine");
    _delay_ms(1000);
    LOG_DEBUG(DEFAULT, "Start BME280 Test");
    bme280_init();
    LOG_DEBUG(DEFAULT, "End BME280 Test");

  }

  // can never be reached
  return 0;
}