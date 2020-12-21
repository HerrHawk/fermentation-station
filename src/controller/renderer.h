#pragma once
#include <avr/io.h>

void display_init();
void display_send_command(uint8_t command);
void display_reset();
extern inline void display_wait_until_idle();
void display_wipe(void);