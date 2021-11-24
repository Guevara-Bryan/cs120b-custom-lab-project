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

#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

#include "SynchSMs.h"

int main(void) {
//=========== PORTS INIT ================
    DDRA = 0x00; PORTA = 0xFF;
    DDRB = 0xFF; PORTB = 0x00;
    DDRC = 0xFF; PORTC = 0x00;
    DDRD = 0xFF; PORTD = 0x00;
//=======================================
    TimerSet(PeriodGCD);
    TimerOn();
    USART_Init(MYUBRR);
    LCD_init();
    LCD_ClearScreen();
    SynchSM_init();
    while (1) {}
    return 1;
}
