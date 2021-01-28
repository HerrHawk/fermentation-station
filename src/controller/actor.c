#include "actor.h"


void setup_heating_element()
{
    //Set Mode to Fast PWM
    TCCR2A |= (1<<WGM20 | 1<<WGM21);

    //Set Clock Source (no Pre Scaler)
    TCCR2B |= (1<<CS20);

    //Set Comper Register for Counter (50% for testing)
    OCR2B = 0x00;

    //Set Pin 3 as output
    DDRD |= 1<<PD3;

    //TCCR2A|=1<<COM2B1;
}


void activate_heating_pwm()
{
    TCCR2A|=1<<COM2B1;
}

void deaktivate_heating_pwm()
{
    TCCR2A&=~(1<<COM2B1);
}

void update_heating_dutycycle(uint8_t dutycyle)
{
    OCR2B = dutycyle;
}

//---------------
//Humidifier
//USB Device cant be controlled with PWM. Only On/Off
//---------------

void setup_humidifier()
{
    DDRD |= 1<<PD4;
}

void activate_humidifier()
{
    PORTD |= (1<<PD4);
}

void deactivate_humidifier()
{
    PORTD &= ~(11<<PD4);
}