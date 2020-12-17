#pragma once
#include <avr/io.h>

void spi_init();
void spi_main_transmit(uint8_t data);
uint8_t spi_main_receive();