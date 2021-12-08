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
#include "joystick.h"
#include "screens.h"
#include "melody.h"
#include <stdio.h>
#include "custom_chars.h"


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
unsigned char numTasks = 11;
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
const unsigned char REFRESH_SCREEN = 0x04;

// 3 different screens with 28 characters. Last 4 are custom characters.
enum MenuScreens { browse, details, options };
Menu main_menu;

// coms. utilities
const char* labels = "123456789ABCDEFGHIJKLMNOPQRSTUVW";
Melody melody;
unsigned char num_melodies = 0;
unsigned char data_communication_code;
//-------------------------------------------

//-------------- Tick functions --------------
int Transmit(int state){
    if((PCR & SEND_DATA)){
        //disable data transmission until set again by another process.
        PCR &= ~SEND_DATA;
        if(data_communication_code == TRANSMISSION_REQUEST){
            serialize_melody(&melody, melody.serialized);

            USART_Transmit(TRANSMISSION_REQUEST);
            unsigned char code_received = USART_Receive();
            if(code_received == TRANSMISSION_ACKNOWLEDGEMENT){
                for(unsigned short i = 0; i < SIZEOFMELODY; i++){
                    USART_Transmit(melody.serialized[i]);
                }
            }
        }
        else if(data_communication_code == EEPROM_LOAD_REQUEST){
            USART_Transmit(EEPROM_LOAD_REQUEST);
            data_communication_code = USART_Receive();
            if(data_communication_code == EEPROM_LOAD_ACKNOWLEDGEMENT){
                USART_Transmit(main_menu.screens[main_menu.current_screen].cursor_pointer.linear - 1);
                PCR |= REFRESH_SCREEN;
                main_menu.current_screen = details;
            }
        }
        else if(data_communication_code == EEPROM_SAVE_REQUEST){
            USART_Transmit(EEPROM_SAVE_REQUEST);
            data_communication_code = USART_Receive();
            if (data_communication_code == EEPROM_SAVE_ACKNOWLEDGEMENT){
                main_menu.current_screen = browse;
                PCR |= REFRESH_SCREEN;
                PCR |= SEND_DATA;
                data_communication_code = EEPROM_META_REQUEST;
            }

        }
        else if(data_communication_code == EEPROM_META_REQUEST){
            USART_Transmit(EEPROM_META_REQUEST);
            data_communication_code = USART_Receive();
            if(data_communication_code == EEPROM_META_ACKNOWLEDGEMENT){
                num_melodies = USART_Receive();
                PCR |= REFRESH_SCREEN;
                main_menu.current_screen = browse;
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
            memset(melody.serialized, 0, SIZEOFMELODY);

            USART_Transmit(TRANSMISSION_ACKNOWLEDGEMENT);
            for(unsigned short i = 0; i < SIZEOFMELODY; i++){
                melody.serialized[i] = USART_Receive();
            }
            melody.is_serialized = 1;
            deserialize_melody(&melody, melody.serialized);
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
    if((PCR & REFRESH_SCREEN)){
        switch(main_menu.current_screen){
            case browse:
                if(num_melodies > 0){
                    for(char i = 0; i < num_melodies; i++){
                        update_char(&main_menu.screens[browse], i, labels[i]);
                    }
                }
                break;
            case details:
                sprintf(main_menu.screens[details].content, "Notes[%02u]-------Time:%-03usec", melody.length, (melody.time_length / 1000));
                for(char i = 0; i < 7; i++){
                    if(melody.notes[i] != silent){
                        update_char(&main_menu.screens[main_menu.current_screen], 9 + i, char_notes[melody.notes[i]]);
                    } else {
                        update_char(&main_menu.screens[main_menu.current_screen], 9 + i, ' ');
                    }
                }
                break;
            case options:
                strcpy(main_menu.screens[options].content, "   1-Save                   ");
                break;
        }
    }
}

int navigateScreen(int state){
    switch(main_menu.current_screen){
        case browse:
            if(JSR & J_LEFT){
                if(main_menu.screens[browse].cursor_pointer.x == 0){
                    previousScreen(&main_menu);
                    cursor_init(&main_menu.screens[main_menu.current_screen].cursor_pointer, 3, 0);
                    PCR |= REFRESH_SCREEN;
                }else{
                    move_left(&main_menu.screens[main_menu.current_screen].cursor_pointer);
                }
            } else if(JSR & J_RIGHT){
                if(main_menu.screens[browse].cursor_pointer.x == screen_width - 1){
                    nextScreen(&main_menu);
                    LCD_Cursor(0); // hide cursor;
                    PCR |= REFRESH_SCREEN;
                }else{
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
                cursor_init(&main_menu.screens[main_menu.current_screen].cursor_pointer, 0, 0);
                PCR |=REFRESH_SCREEN;
            } else if(JSR & J_RIGHT){
                nextScreen(&main_menu);
                cursor_init(&main_menu.screens[main_menu.current_screen].cursor_pointer, 3, 0);
                PCR |= REFRESH_SCREEN;
            } else if(JSR & J_UP){
                move_up(&main_menu.screens[main_menu.current_screen].cursor_pointer);
            } else if(JSR & J_DOWN){
                move_down(&main_menu.screens[main_menu.current_screen].cursor_pointer);
            }
            break;
        case options:
            if(JSR & J_LEFT){
                previousScreen(&main_menu);
                LCD_Cursor(0); // hide cursor
                PCR |= REFRESH_SCREEN;
            } else if(JSR & J_RIGHT){
                nextScreen(&main_menu);
                cursor_init(&main_menu.screens[main_menu.current_screen].cursor_pointer, 0, 0);
                PCR |=REFRESH_SCREEN;
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

enum saveStates{ Sstart, Swait, Ssave, Shold };
int saveMelodySM(int state){
    if(main_menu.current_screen == options){
        unsigned char input = (JSR & J_SELECT);
        switch(state){
            case Sstart:
                state = Swait;
                break;
            case Swait:
                state = input ? Ssave: Swait;
                break;
            case Ssave:
                PCR |= SEND_DATA;
                data_communication_code = EEPROM_SAVE_REQUEST;
                state = Shold;
                break;
            case Shold:
                state = input ? Shold : Swait;
                if(state == Swait){
                    PCR |= REFRESH_SCREEN;
                    main_menu.current_screen = details;
                }
                break;
        }
    }
    return state;
}

enum getStates{ Gstart, Gwait, Gget, Ghold };
int getMelodySM(int state){
    if(main_menu.current_screen == browse){
        unsigned char input = (JSR & J_SELECT);
        switch(state){
            case Gstart:
                state = Gwait;
                break;
            case Gwait:
                state = input ? Gget : Gwait;
                break;
            case Gget:
                PCR |= SEND_DATA;
                data_communication_code = EEPROM_LOAD_REQUEST;
                state = Ghold;
                break;
            case Ghold:
                state = input ? Ghold : Gwait;
                if(state == Gwait){
                    PCR |= REFRESH_SCREEN;
                    main_menu.current_screen = details;
                }
                break;
        }
    }
    return state;
}


enum ReplayStates{RStart, rwait, rsend, rhold };
int replayMelodySM(int state){
    unsigned char input = (JSR & J_SELECT);
    if(main_menu.current_screen == details){
        switch(state){
            case RStart:
                state = rwait;
                break;
            case rwait:
                state = input ? rsend : rwait;        
                break;
            case rsend:
                state = rhold;
                PCR |= SEND_DATA;
                data_communication_code = TRANSMISSION_REQUEST;
                break;
            case rhold:
                state = input ? rhold : rwait;
                break;
            default:
                state = rwait;
                break;
        }
    }
    return state;
}

int DisplayScreen(int state){
    if((PCR & REFRESH_SCREEN)){
        PCR &= ~REFRESH_SCREEN;
        LCD_ClearScreen();
        LCD_DisplayString(1, main_menu.screens[main_menu.current_screen].content);
        LCD_LoadCustomCharacter(browse_light, 0);
        LCD_LoadCustomCharacter(details_light, 1);
        LCD_LoadCustomCharacter(options_light, 2);
        LCD_LoadCustomCharacter(browse_dark, 3);
        LCD_LoadCustomCharacter(details_dark, 4);
        LCD_LoadCustomCharacter(options_dark, 5);


        for(unsigned char i = 0; i < 3; i++){
            if(i == main_menu.current_screen){
                LCD_WriteCustomCharacter(i + 3, 30 + i);
            } else {
                LCD_WriteCustomCharacter(i, 30 + i);
            }
        }
    }
    LCD_Cursor(main_menu.screens[main_menu.current_screen].cursor_pointer.linear);
    return state;
}
//--------------------------------------------

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
    tasks[i].Tick = &navigateScreen;
    i++;  
    tasks[i].state = generic_start;
    tasks[i].period = PeriodGCD;
    tasks[i].elapsedTime = tasks[i].period;
    tasks[i].Tick = &updateScreen;
    i++;  
    tasks[i].state = generic_start;
    tasks[i].period = PeriodGCD;
    tasks[i].elapsedTime = tasks[i].period;
    tasks[i].Tick = &saveMelodySM;
    i++;  
    tasks[i].state = generic_start;
    tasks[i].period = PeriodGCD;
    tasks[i].elapsedTime = tasks[i].period;
    tasks[i].Tick = &getMelodySM;
    i++; 
    tasks[i].state = generic_start;
    tasks[i].period = PeriodGCD;
    tasks[i].elapsedTime = tasks[i].period;
    tasks[i].Tick = &replayMelodySM;
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