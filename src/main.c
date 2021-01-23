#include "controller/renderer.h"
#include "globals.h"
#include "helpers.h"
#include "logging.h"
#include <avr/io.h>
#include <util/delay.h>

int main(void)
{
  // run setup functions
  uart_init();
  display_init();

  while (1) {
    demo();
  }

  // can never be reached
  return 0;
}