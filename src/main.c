#include <avr/io.h>
#include <util/delay.h>
#include "helpers.h"

int main(void)
{
    // runce once
    // set which pins to use as output in the Data Direction Register
    DDRB = (1 << PB5);

    //switch on LEDs
    while (1)
    {
        setBit(PORTB, PB5);
        _delay_ms(200);
        clearBit(PORTB, PB5);
        _delay_ms(200);
        toggleBit(PORTB, PB5);
        _delay_ms(200);
        toggleBit(PORTB, PB5);
        _delay_ms(1000);
    }
    // can never be reached
    return 0;

}