#pragma once
#include <avr/io.h>

#define SS PB1   // Slave Select -> in our case always the eInk display
#define MOSI PB2 // Master Out Slave In -> used to set direction
#define MISO PB3 // Master In Slave Out -> used to set direction
#define SCK PB5  // Shared Clock Signal -> shared because of synchronous communication

void spi_init();
void spi_main_transmit(uint8_t data);
uint8_t spi_main_receive();