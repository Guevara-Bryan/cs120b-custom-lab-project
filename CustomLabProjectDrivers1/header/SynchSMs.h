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
#include "io.h"
#include <stdlib.h>


//============== SynchSMs setup ==============
task* tasks; // array of size numTasks
unsigned char numTasks = 1;
unsigned short PeriodGCD = 1000;
//============================================

//------------ Global Variables -------------

//-------------------------------------------

//-------------- Tick functions --------------
int BlinkLedSM(int state){
    PORTA = !PORTA;

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
    tasks[i].Tick = &BlinkLedSM;
    i++;

    free(tasks);    
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