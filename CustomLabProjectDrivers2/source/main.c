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
#include "io.h"
#include "SynchSMs.h"

#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

int main(void) {
//=========== PORTS INIT ================
    DDRA = 0xFF; PORTA = 0x00;
//=======================================
    TimerSet(PeriodGCD);
    TimerOn();
    SynchSM_init();
    while (1) {}
    return 1;
}
