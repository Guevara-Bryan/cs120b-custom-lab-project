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
#include "usart.h"
#include "io.h"
#include "melody.h"
#include <stdlib.h>


//============== SynchSMs setup ==============
task* tasks; // array of size numTasks
unsigned char numTasks = 2;
unsigned short PeriodGCD = 250;
//============================================

//------------ Global Variables -------------
unsigned char expecting_data = 0;
unsigned char data[MAX_NOTES];
const unsigned char* notes = "SCDEFGAB*";
//-------------------------------------------

//-------------- Tick functions --------------
int Receive(int state){
    //expecting_data = 1 whenever we are on recording mode.
    if(expecting_data){      
        LCD_DisplayString(17, "...Receiving");
        // block data reception for next cycle
        expecting_data = 0;
        //Flush the data buffer.
        USART_FlusDataBuffer();
        unsigned char i = 0;
        while(i < MAX_NOTES){
            data[i] = USART_Receive();
            if (data[i] == 0) { break; } // End of transmission
            i++;
        }

        //Display data on LED screen
        LCD_DisplayString(17, "-->Done!<--");
        i = 0;
        while (i < 16 && data[i] != 0){
            LCD_Cursor(i + 1);
            LCD_WriteData(notes[data[i]]);
            i++;
        }
    }
    
    return state;
}

enum EnableStates {start, string, hold, reset};
int EnableExpectingData(int state){
    unsigned char input = ~PINA & 0x04;// Get the input from the joystick button
    switch (state)
    {
        case start:
            state = string;
            break;
        case string:
            LCD_DisplayString(17, "<--No Data-->");
            state = hold;
            break;
        case hold:
            if(input){
                LCD_ClearScreen();
                expecting_data = 1;
                state = reset;
            }
            break;
        case reset:
            if(input){
                state = start;
            }
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
    tasks[i].Tick = &Receive;
    i++;  
    tasks[i].state = start;
    tasks[i].period = PeriodGCD;
    tasks[i].elapsedTime = tasks[i].period;
    tasks[i].Tick = &EnableExpectingData;
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