#pragma once
#include <stdio.h>

void uart_putchar(char c, FILE* stream);
char uart_getchar(FILE* stream);

void uart_init(void);

/* http://www.ermicro.com/blog/?p=325 */

FILE uart_output;
FILE uart_input;

typedef enum
{
  DEFAULT,
  TEMPERATURE,
  HUMIDITY,
  DISPLAY,
  CONTROL,
  I2C,
  SPI,
} LogSector;

const int const sectorLoggingEnabled[];
const char* const sectorNames[];

#ifdef DEBUG
#define LOG_DEBUG(sector, message, ...)                                                            \
  sectorLoggingEnabled[sector]                                                                     \
    ? fprintf(                                                                                     \
        &uart_output, "%12s [%s:%d] | %s\n", sectorNames[sector], __FILE__, __LINE__, message)     \
    : 0
#else
#define LOG_DEBUG(sector, message)                                                                 \
  do {                                                                                             \
  } while (0)
#endif
