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
unsigned char numTasks = 8;
unsigned short PeriodGCD = 250;
//============================================

//------------ Global Variables -------------
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

/*
    PCR - Process Control Register
    Allow you to set flags that let you control which processes the
    program can execute.

*/
unsigned char PCR = 0x00;
const unsigned char SEND_DATA = 0x01;
const unsigned char RECEIVE_DATA = 0x02;
const unsigned char REFRESH_SCREEN = 0x04;


// 4 different screens with 28 characters. Last 4 are custom characters.
enum MenuScreens { receive, browse, details, options };
Menu main_menu;

Melody melody;
//-------------------------------------------

//-------------- Tick functions --------------
int Transmit(int state){
    if((PCR & SEND_DATA)){
        //disable data transmission until set again by another process.
        PCR &= ~SEND_DATA;

        if(!melody.is_serialized) { serialize_melody(&melody, melody.serialized); }

        USART_Transmit(TRANSMISSION_REQUEST);
        unsigned char code_received = USART_Receive();
        if(code_received == TRANSMISSION_ACKNOWLEDGEMENT){
            for(unsigned short i = 0; i < SIZEOFMELODY; i++){
                USART_Transmit(melody.serialized[i]);
            }
        }
    }
    return state;
}

int Receive(int state){
    unsigned char code_received;
    if(USART_CheckReceiveFlag()){
        code_received = USART_Receive();
        if(code_received == TRANSMISSION_REQUEST){      
            reset_melody(&melody);
            memset(melody.serialized, 0, SIZEOFMELODY);

            USART_Transmit(TRANSMISSION_ACKNOWLEDGEMENT);
            for(unsigned short i = 0; i < SIZEOFMELODY; i++){
                melody.serialized[i] = USART_Receive();
            }
            melody.is_serialized = 1;
            deserialize_melody(&melody, &melody.serialized);
            scroll_down(&main_menu, receive);
            PCR |= REFRESH_SCREEN;
        }
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

int updateScreen(int state){
    switch(main_menu.current_screen){
        case receive:
            break;
        case browse:
            break;
        case details:
            sprintf(main_menu.screens[main_menu.current_screen].content, "Notes[%02u]:ABGCD>Time:%-03usec", melody.length, (melody.time_length / 1000));
            for(char i = 0; i < 5; i++){
                if(melody.notes[i] != silent){
                    update_char(&main_menu.screens[main_menu.current_screen], 10 + i, char_notes[melody.notes[i]]);
                }
            }
            PCR |= REFRESH_SCREEN;
            break;
        case options:
            break;
        default:
            break;
    }
}

int navigateScreen(int state){
    switch(main_menu.current_screen){
        case receive: // This screen shows you whether a melody has been received or not
            if(JSR & J_LEFT){
                previousScreen(&main_menu);
                PCR |= REFRESH_SCREEN;
            } else if(JSR & J_RIGHT){
                nextScreen(&main_menu);
                PCR |= REFRESH_SCREEN;
            }
            break;
        case browse:
            if(JSR & J_LEFT){
                if(main_menu.screens[main_menu.current_screen].cursor_pointer.x == 0){
                    previousScreen(&main_menu);
                    PCR |= REFRESH_SCREEN;
                } else {
                    move_left(&main_menu.screens[main_menu.current_screen].cursor_pointer);
                }
            } else if(JSR & J_RIGHT){
                if(main_menu.screens[main_menu.current_screen].cursor_pointer.x == (screen_width-1)){
                    nextScreen(&main_menu);
                    PCR |= REFRESH_SCREEN;
                } else {
                    move_right(&main_menu.screens[main_menu.current_screen].cursor_pointer);
                }
            } else if(JSR & J_UP){
                move_up(&main_menu.screens[main_menu.current_screen].cursor_pointer);
            } else if(JSR & J_DOWN){
                move_down(&main_menu.screens[main_menu.current_screen].cursor_pointer);
            }
            break;
        case details:
            if(JSR & J_LEFT){
                previousScreen(&main_menu);
                PCR |= REFRESH_SCREEN;
            } else if(JSR & J_RIGHT){
                nextScreen(&main_menu);
                PCR |= REFRESH_SCREEN;
            } else if(JSR & J_UP){
                move_up(&main_menu.screens[main_menu.current_screen].cursor_pointer);
            } else if(JSR & J_DOWN){
                move_down(&main_menu.screens[main_menu.current_screen].cursor_pointer);
            }
            break;
        case options:
            if(JSR & J_LEFT){
                if(main_menu.screens[main_menu.current_screen].cursor_pointer.x == 0){
                    previousScreen(&main_menu);
                    PCR |= REFRESH_SCREEN;
                } else {
                    move_left(&main_menu.screens[main_menu.current_screen].cursor_pointer);
                }
            } else if(JSR & J_RIGHT){
                if(main_menu.screens[main_menu.current_screen].cursor_pointer.x == (screen_width-1)){
                    nextScreen(&main_menu);
                    PCR |= REFRESH_SCREEN;
                } else {
                    move_right(&main_menu.screens[main_menu.current_screen].cursor_pointer);
                }
            } else if(JSR & J_UP){
                move_up(&main_menu.screens[main_menu.current_screen].cursor_pointer);
            } else if(JSR & J_DOWN){
                move_down(&main_menu.screens[main_menu.current_screen].cursor_pointer);
            }
            break;
        default:
            break;
    }
    return state;
}

int DisplayScreen(int state){
    if((PCR & REFRESH_SCREEN)){
        PCR &= ~REFRESH_SCREEN;
        LCD_DisplayString(1, getCurrentScreenContent(&main_menu, main_menu.screens[main_menu.current_screen].frame));
    }
    LCD_Cursor(main_menu.screens[main_menu.current_screen].cursor_pointer.linear);
    return state;
}
//--------------------------------------------

// Load the different screens into the menu
void Menu_load(){
    Screen_init(&main_menu.screens[receive], 0, "  ...Receiving              ");
    update_screen(&main_menu, receive, 1,       "  Data Received             ");
    Screen_init(&main_menu.screens[browse],  0, "123456789ABCDEFGHIJKLMNOPQR>");
    Screen_init(&main_menu.screens[details], 0, "Notes[%u]:ABGCD>Time:%usec");
    Screen_init(&main_menu.screens[options], 0, "  Play  Delete    Stop  save");
}


//---------------- Initialize SMS ----------------
void SynchSM_init(){
    tasks = (task*)calloc(numTasks, sizeof(task));
    unsigned char i = 0;
    const unsigned char generic_start = 0;

    tasks[i].state = generic_start;
    tasks[i].period = PeriodGCD;
    tasks[i].elapsedTime = tasks[i].period;
    tasks[i].Tick = &Transmit;
    i++;  
    tasks[i].state = generic_start;
    tasks[i].period = PeriodGCD;
    tasks[i].elapsedTime = tasks[i].period;
    tasks[i].Tick = &Receive;
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
    tasks[i].Tick = &updateScreen;
    i++;  
    tasks[i].state = generic_start;
    tasks[i].period = PeriodGCD;
    tasks[i].elapsedTime = tasks[i].period;
    tasks[i].Tick = &navigateScreen;
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