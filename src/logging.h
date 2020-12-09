#pragma once
#include <stdio.h>

// typedef enum
// {
//   INFO,
//   DEBUG,
//   WARNING,
//   ERROR
// } LOG_LEVEL;

// void logg(LOG_LEVEL level, char* msg);
void uart_putchar(char c, FILE* stream);
char uart_getchar(FILE* stream);
void uart_init(void);

FILE uart_output = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);
FILE uart_input = FDEV_SETUP_STREAM(NULL, uart_getchar, _FDEV_SETUP_READ);
// FILE uart_io = FDEV_SETUP_STREAM(uart_putchar, uart_getchar, _FDEV_SETUP_RW);