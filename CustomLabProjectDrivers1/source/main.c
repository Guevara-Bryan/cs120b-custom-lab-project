/*	Author: Bryan Guevara
 *  Partner(s) Name: 
 *	Lab Section:
 *	Assignment: Lab #  Exercise #
 *	Exercise Description: [optional - include for your own benefit]
 *  Video Demo: 
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
#include "SynchSMs.h"

#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

int main(void) {
//=========== PORTS INIT ================
    DDRA = 0xFF; PORTA = 0x00;
    DDRB = 0xF0; PORTB = 0x0F;
    DDRC = 0xFF; PORTC = 0x00;
    DDRD = 0xFF; PORTD = 0x00;
//=======================================
    TimerSet(PeriodGCD);
    TimerOn();
    USART_Init(MYUBRR);
    reset_melody(&melody);
    PWM_on();
    SynchSM_init();
    while (1) {}
    return 1;
}
