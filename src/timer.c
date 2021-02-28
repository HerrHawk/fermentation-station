#include "timer.h"
#include "avr/io.h"
#include "avr/interrupt.h"

volatile uint8_t s1_triggered = 0;

void setup_timer_s1()
{   
    
    OCR1A = 0x2DC6;  //1 sec
    //OCR1A = 0x5B8C;  //2 sec
    //OCR1A = B71B; //4 sec


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
    //"event" will be consumed after sensor data is read out
    s1_triggered = 1;

}