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
#include <stdlib.h>


//============== SynchSMs setup ==============
task* tasks; // array of size numTasks
unsigned char numTasks = 2;
unsigned short PeriodGCD = 300;
//============================================

//------------ Global Variables -------------
unsigned char expecting_data = 0;
unsigned char* data;
const unsigned char notes[9] = "SCDEFGAB1"; //Just the notes I am using
//-------------------------------------------

//-------------- Tick functions --------------
int ReceiveChar(int state){
    //expecting_data = 1 whenever we are on recording mode.
    if(expecting_data){
        LCD_Cursor(16);
        LCD_WriteData('W');
        //Dissable data reception until set again by another process.
        expecting_data = 0;
        //Get length of the data to be received
        unsigned char data_length = USART_Receive();
        // Only begin to receive data if the length is not zero
        if(data_length > 0){
            data = (unsigned char*) calloc(data_length, sizeof(char));
            for(unsigned char i = 0; i < data_length; i++){
                data[i] = USART_Receive();
            }

            for(unsigned char i = 0; i < data_length; i++){
                LCD_Cursor(i + 1);
                LCD_WriteData(notes[data[i]]);
            }
        } 
    }
    //Note free() data once no longer needed.
    return state;
}

int EnableExpectingData(int state){
    LCD_Cursor(16);
    LCD_WriteData('N');
    unsigned char input = ~PINA & 0x04;// Get the input from the joystick button
    if(input){
        LCD_ClearScreen();
        expecting_data = 1;
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
    tasks[i].Tick = &ReceiveChar;
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