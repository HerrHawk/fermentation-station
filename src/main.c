#include "globals.h"
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
  bme280_init();

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
    int32_t temp =  bme280_read_temp();
    LOG_DEBUG(DEFAULT, "Temperature: %ld", temp);
    LOG_DEBUG(DEFAULT, "End BME280 Test");

    _delay_ms(100);

  }

  // can never be reached
  return 0;
}