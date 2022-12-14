#pragma once
#include <avr/io.h>
#include "../logging.h"


void setup_heating_element();
void activate_heating_pwm();
void deaktivate_heating_pwm();
void update_heating_dutycycle(uint8_t dutycyle);

void setup_humidifier();
void activate_humidifier();
void deactivate_humidifier();