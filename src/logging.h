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
  I2C_DEEP,
  SPI,
} LogSector;

const int const sector_logging_enabled[];
const char* const sector_names[];

#define log(...) fprintf(&uart_output, __VA_ARGS__)

#ifdef DEBUG
#define LOG_DEBUG(sector, ...)                                                                     \
  do {                                                                                             \
    if (sector_logging_enabled[sector]) {                                                          \
      fprintf(&uart_output, __VA_ARGS__);                                                          \
    }                                                                                              \
  } while (0)
#else
#define LOG_DEBUG(sector, ...)                                                                     \
  do {                                                                                             \
  } while (0)
#endif
