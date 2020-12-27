#include "spi.h"
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
  // set SS, MOSI and SCK as output
  // MISO is already set as input
  DDRB |= _BV(SS) | _BV(MOSI) | _BV(SCK);

  // enable SPI | define as main
  SPCR |= _BV(SPE) | _BV(MSTR);

  // set clock rate : SPR0:=1 SPR1:=0 --> SPR1hfOSC/16
  SPCR |= _BV(SPR0);

  // set MSB first
  SPCR &= ~_BV(DORD);

  LOG_DEBUG(SPI, "SPI successfully initialized");
}

/**
 * Transmit data from main to node.
 *
 * @param datas The data to be transmitted
 */
void spi_main_transmit(uint8_t data)
{
  // Before data can be transmitted, the slave select line (SS) needs to be set to low
  PORTB &= ~_BV(SS);

  // load data to be transmitted into register
  SPDR = data;

  // Wait until data has been transmitted
  while (!(SPSR & _BV(SPIF))) {};

  // reset SS after transmit
  PORTB |= _BV(SS);
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
