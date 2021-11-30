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
#include "joystick.h"
#include "screens.h"
#include <stdlib.h>
#include <stdio.h>

//Joystick definitions
#define J_LEFT 0x01
#define J_RIGHT 0x02
#define J_UP 0x04
#define J_DOWN 0x08
#define J_SELECT 0X10

#define TOP_ACTIVATION_THRESHOLD 600
#define BOTTOM_ACTIVATION_THRESHOLD 500

//============== SynchSMs setup ==============
task* tasks; // array of size numTasks
unsigned char numTasks = 7;
unsigned short PeriodGCD = 250;
//============================================

//------------ Global Variables -------------
unsigned char expecting_data = 0;
unsigned char refresh_screen = 1;
unsigned char load_melody = 0;
//melody data buffer
unsigned char data[SIZEOFMELODY];
//Joystick
/*  Joystick Status Register
 *  Bit 0 - Left
 *  Bit 1 - Right
 *  Bit 2 - Up
 *  Bit 3 - Down 
 *  Bit 4 - Select Button
 *  JSR = 0x00 - Do nothing
 */
unsigned char JSR;


// 4 different screens with 28 characters. Last 4 are custom characters.
enum MenuScreens { receive, browse, details, options };
Menu main_menu;

Melody melody;
//-------------------------------------------

//-------------- Tick functions --------------
int Receive(int state){
    //expecting_data = 1 whenever we are on recording mode.
    if(expecting_data){      
        // block data reception for next cycle
        expecting_data = 0;
        //Flush the data buffer.
        USART_FlusDataBuffer();

        for(unsigned short i = 0; i < SIZEOFMELODY; i++){
            data[i] = USART_Receive();
        }

        load_melody = 1;
        main_menu.current_screen = details;
    }
    return state;
}

int InterpretData(int state){
    if (load_melody){
        load_melody = 0;

        reset_melody(&melody);

        memcpy(melody.notes, &data[NOTES_OFFSET], MAX_NOTES);
        melody.length = data[0];
        melody.time_length = data[2];
        melody.time_length <<= 8;
        melody.time_length |= data[1];

        refresh_screen = 1;
    }

    return state;
}

int GetJoystickX(int state){
    unsigned short input;
    ADC_change_channel(HORIZONTAL_CHANNEL);
    input = ADC;
    if(input >= TOP_ACTIVATION_THRESHOLD){
        JSR |= J_RIGHT;
    }else if(input <= BOTTOM_ACTIVATION_THRESHOLD){
        JSR |= J_LEFT;
    }else { 
        JSR &= ~J_LEFT;
        JSR &= ~J_RIGHT;
    }
    return state;
}

int GetJoystickY(int state){
    unsigned short input;
    ADC_change_channel(VERTICAL_CHANNEL);
    input = ADC;
    if(input >= TOP_ACTIVATION_THRESHOLD){
        JSR |= J_UP;
    }else if(input <= BOTTOM_ACTIVATION_THRESHOLD){
        JSR |= J_DOWN;
    }else { 
        JSR &= ~J_UP;
        JSR &= ~J_DOWN;
    }
    return state;
}

int GetButton(int state){
    unsigned char input = ~PINA & 0x04;

    if(input){
        JSR |= J_SELECT;
    } else {
        JSR &= ~J_SELECT;
    }

    return state;
}


int UpdateScreen(int state){

    if ((JSR & J_LEFT)){
        previousScreen(&main_menu);
        refresh_screen = 1;
    } else if ((JSR & J_RIGHT)){
        nextScreen(&main_menu);
        refresh_screen = 1;
    }
    if(main_menu.current_screen == receive){
        expecting_data = 1;
    }

    if(refresh_screen){
        switch(main_menu.current_screen){
            case browse:
                break;
            case details:
                sprintf(main_menu.screens[main_menu.current_screen].content, "Notes[%02u]:ABGCD>Time:%-3usec", melody.length, (melody.time_length/1000));
                for(char i = 0; i < 5; i++){
                    if(melody.notes[i] == silent){
                        update_char(&main_menu.screens[main_menu.current_screen], 10 + i, ' ');
                    } else {
                        update_char(&main_menu.screens[main_menu.current_screen], 10 + i, char_notes[melody.notes[i]]);
                    }
                }
                break;
            case options:
                break;
            default:
                break;
        }
    }
    return state;
}

int DisplayScreen(int state){
    if(refresh_screen){
        refresh_screen = 0;
        LCD_DisplayString(1, main_menu.screens[main_menu.current_screen].content);
        LCD_Cursor(main_menu.screens[main_menu.current_screen].cursor_pos);
    }
    return state;
}
//--------------------------------------------

// Load the different screens into the menu
void Menu_load(){
    Screen_init(&main_menu.screens[receive], 32, "  ...Receiving              ");
    Screen_init(&main_menu.screens[browse], 32, "123456789ABCDEFGHIJKLMNOPQR>");
    Screen_init(&main_menu.screens[details], 32, "Notes[%u]:ABGCD>Time:%usec");
    Screen_init(&main_menu.screens[options], 32, "  Play  Delete    Stop  save");
}


//---------------- Initialize SMS ----------------
void SynchSM_init(){
    tasks = (task*)calloc(numTasks, sizeof(task));
    unsigned char i = 0;
    const unsigned char generic_start = 0;

    tasks[i].state = generic_start;
    tasks[i].period = PeriodGCD;
    tasks[i].elapsedTime = tasks[i].period;
    tasks[i].Tick = &Receive;
    i++;  
    tasks[i].state = generic_start;
    tasks[i].period = PeriodGCD;
    tasks[i].elapsedTime = tasks[i].period;
    tasks[i].Tick = &InterpretData;
    i++;  
    tasks[i].state = generic_start;
    tasks[i].period = PeriodGCD;
    tasks[i].elapsedTime = tasks[i].period;
    tasks[i].Tick = &GetJoystickX;
    i++;  
    tasks[i].state = generic_start;
    tasks[i].period = PeriodGCD;
    tasks[i].elapsedTime = tasks[i].period;
    tasks[i].Tick = &GetJoystickY;
    i++;  
    tasks[i].state = generic_start;
    tasks[i].period = PeriodGCD;
    tasks[i].elapsedTime = tasks[i].period;
    tasks[i].Tick = &GetButton;
    i++;  
    tasks[i].state = generic_start;
    tasks[i].period = PeriodGCD;
    tasks[i].elapsedTime = tasks[i].period;
    tasks[i].Tick = &UpdateScreen;
    i++;  
    tasks[i].state = generic_start;
    tasks[i].period = PeriodGCD;
    tasks[i].elapsedTime = tasks[i].period;
    tasks[i].Tick = &DisplayScreen;
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