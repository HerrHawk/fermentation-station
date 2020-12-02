#include <avr/io.h>
#include <util/delay.h>

int main(void)
{
    // runce once
    // configure LED pins as output
    // configure all other pins as input
    DDRB = (1 << PB5);

    //switch on LEDs
    // PB0 - PB2 HIGH
    while (1)
    {
        // 0b10101010 &
        PORTB = 0b00100000;
        _delay_ms(500);
        PORTB = 0b00000000;
        _delay_ms(100);
        PORTB = 0b00100000;
        _delay_ms(500);
        PORTB = 0b00000000;
        _delay_ms(1000);
    }
    // can never be reached
    return 0;

}