/*	Author: Bryan Guevara
 *  Partner(s) Name: 
 *	Lab Section:
 *	Assignment: Lab 11  Exercise 5
 *	Exercise Description: Obstacle avoiding game
 *  Video Demo: 
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */

#ifndef SYNCHSM_H
#define SYNCHSM_H
#include "task.h"
#include <avr/io.h>
#include "timer.h"
#include "pwm.h"
#include "usart.h"
#include "melody.h"
#include <stdlib.h>



//============== SynchSMs setup ==============
task* tasks; // array of size numTasks
unsigned char numTasks = 3;
unsigned short PeriodGCD = 100;
//============================================

//------------ Global Variables -------------
unsigned char send_data = 0;
unsigned short recording = 0;

Melody melody;
int current_note = silent;
unsigned short current_start = 0;
unsigned short current_duration = 0;
//-------------------------------------------

//-------------- Tick functions --------------
int Transmit(int state){
    if(send_data){
        //disable data transmission until set again by another process.
        send_data = 0;
        //Turn on LED on PA1
        PORTA = 0x03;
        //Send the length of the input
        USART_Transmit(melody.length);
        //Wait for data to be Transmitted
        USART_WaitForTransmit();
        //Send the whole data block
        for(unsigned char i = 0; i < melody.length; i++){
            USART_Transmit(melody.notes[i]);
            USART_WaitForTransmit();
        }
        //clear the melody to start again
        reset_melody(&melody);
        PORTA = 0x00;
    }
    return state;
}

int NoteEnterSM(int state){
    unsigned char input = ~PINB & 0x0F;
    switch(input){
        case 0x01:
            current_note = C;
            set_PWM(NOTES[current_note]);
            if(current_start == 0){ current_start = melody.time_length; }
            PORTA |= 0x01;
            break;
        case 0x02:
            current_note = D;
            set_PWM(NOTES[current_note]);
            if(current_start == 0){ current_start = melody.time_length; }
            PORTA |= 0x01;
            break;
        case 0x04:
            current_note = E;
            set_PWM(NOTES[current_note]);
            if(current_start == 0){ current_start = melody.time_length; }
            PORTA |= 0x01;
            break;
        case 0x06:
            recording = 1;
            PORTA |= 0x02;
            break;
        case 0x08:
            current_note = F;
            set_PWM(NOTES[current_note]);
            if(current_start == 0){ current_start = melody.time_length; }
            PORTA |= 0x01;
            break;
        case 0x0F: // Transmit Data **Disables Multitasking**
            send_data = 1;
            recording = 0;
            PORTA &= 0xFD; //Clear recording bit
        default:
            PORTA &= 0xFE;
            if(current_note != silent){
                current_duration = melody.time_length - current_start;
                add_note(&melody, current_note, current_start, current_duration);
            }
            current_note = silent;
            set_PWM(NOTES[current_note]);
            current_start = 0;
            current_duration = 0;
            break;
    }

    return state;
}

int updateMelodyLength(int state){
    if (recording){
        melody.time_length += PeriodGCD;
    }
    return state;
}
//--------------------------------------------

//---------------- Initialize SMS ----------------
void SynchSM_init(){
    tasks = (task*)calloc(numTasks, sizeof(task));
    unsigned char i = 0;
    const unsigned char start = 0;

    tasks[i].state = start;
    tasks[i].period = PeriodGCD;
    tasks[i].elapsedTime = tasks[i].period;
    tasks[i].Tick = &Transmit;
    i++;
    tasks[i].state = start;
    tasks[i].period = PeriodGCD;
    tasks[i].elapsedTime = tasks[i].period;
    tasks[i].Tick = &NoteEnterSM;
    i++;
    tasks[i].state = start;
    tasks[i].period = PeriodGCD;
    tasks[i].elapsedTime = tasks[i].period;
    tasks[i].Tick = &updateMelodyLength;
    i++;
}
//------------------------------------------------

//################# Scheduler ####################
void TimerISR()
{
    unsigned char i;
    for (i = 0; i < numTasks; ++i)
    { // Heart of the scheduler code
        if (tasks[i].elapsedTime >= tasks[i].period)
        { // Ready
            tasks[i].state = tasks[i].Tick(tasks[i].state);
            tasks[i].elapsedTime = 0;
        }
        tasks[i].elapsedTime += PeriodGCD;
    }
}
#endif