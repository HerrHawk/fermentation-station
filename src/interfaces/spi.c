#include "../globals.h"
#include "../helpers.h"
#include "../logging.h"
#include <avr/io.h>

#define SS PB2   // Slave Select -> in our case always the eInk display
#define MOSI PB3 // Master Out Slave In -> used to set direction
#define MISO PB4 // Master In Slave Out -> used to set direction
#define SCK PB5  // Shared Clock Signal -> shared because of synchronous communication

/**
 * Initialize the SP Interface
 *
 * Run this once at startup. Sets all required Bits for communication.
 */
void spi_init()
{
  // set CS, MOSI and SCK as output
  // MISO is already set as input
  DDRB |= _BV(SS) | _BV(MOSI) | _BV(SCK);

  // enable SPI | define as main | set to slowest clock rate: F_osc/128
  SPCR = _BV(SPE) | _BV(MSTR) | _BV(SPR0) | _BV(SPR1);

  LOG_DEBUG(SPI, "SPI successfully initialized");
}

/**
 * Transmit data from main to node.
 *
 * Before using this function, the slave select line (SPI_DDR) needs to be set to low,
 * after transmit back to high.
 *
 * @param datas The data to be transmitted
 */
void spi_main_transmit(uint8_t data)
{
  // load data to be transmitted into register
  SPDR = data;

  // Wait until data has been transmitted
  while (!(SPSR & _BV(SPIF))) {};
}

/**
 * Receive data from node
 *
 * As we only use a display, this will likely never happen. Included just in case.
 *
 * @returns register in which the data is stored (SPDR)
 */
uint8_t spi_main_receive()
{
  // clear SPDR via dummy byte
  SPDR = 0xFF;

  // wait until data has been received
  while (!(SPSR & _BV(SPIF))) {};

  // return the data register
  return SPDR;
}
