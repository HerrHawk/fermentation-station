#include "timer.h"
#include "avr/io.h"
#include "avr/interrupt.h"

volatile uint8_t s1_triggered = 0;

void setup_timer_s1()
{
    OCR1A = 0x2DC6;

    //Mode 4, CTC 
    TCCR1B|=(1<<WGM12);

    //Set interrupt on compare match
    TIMSK1 |= (1<<OCIE1A);

    //Set prescaler to 1024
    TCCR1B |= (1<<CS12)|(1<<CS10);

    sei();
}

ISR(TIMER1_COMPA_vect)
{
    //action for timer
    s1_triggered = 1;

}