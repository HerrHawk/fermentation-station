#include "interfaces/spi.h"

// PORTS on the display
#define RST_PIN PB0
#define DC_PIN PB1
#define CS_PIN PB2
#define BUSY_PIN PD6

unsigned char image[1024];

void init_display()
{
  // Set CS, DC and RST Pins to OUTPUT
  PORTB |= _BV(CS_PIN) | _BV(DC_PIN);
  PORTD |= _BV(RST_PIN);
  // Set BUSY Pin to INPUT
  PORTD &= ~_BV(BUSY_PIN);

  // Initialize SPI Connection to spi.c
  spi_init();
}

void render_image(unsigned char image[]) {}

void clear_display() {}