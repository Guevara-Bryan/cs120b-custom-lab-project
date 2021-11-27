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

#define LED_PORT PORTB
#define NOTES_PORT PINA
#define CTRL_PORT PINC

#define RECORD_BUTTON 0x01
#define SEND_DATA_BUTTON 0x02
#define REPLAY_BUTTON 0x04

#define NOTES_LED 0x01
#define RECORDING_LED 0x04
#define REPLAY_LED 0x02
#define SEND_LED 0x07



//============== SynchSMs setup ==============
task* tasks; // array of size numTasks
unsigned char numTasks = 7;
unsigned short PeriodGCD = 100;
//============================================

//------------ Global Variables -------------
enum Status{recording, send_data, replay};
unsigned char status[3] = {0, 0, 0};

Melody melody;
unsigned char current_note = silent;
unsigned short current_start = 0;
unsigned short current_duration = 0;
//-------------------------------------------

//-------------- Tick functions --------------
int Transmit(int state){
    if(status[send_data]){
        //disable data transmission until set again by another process.
        status[send_data] = 0;
        //Send the data block
        for(unsigned char i = 0; i < melody.length; i++){
            USART_Transmit(melody.notes[i]);
            if (melody.notes[i] == 0){ break; } //Something went wrong
        }
        // Signal end of data transfer
        USART_Transmit(0);
    }
    return state;
}

int PlayNoteSM(int state){
    unsigned char input = ~NOTES_PORT;
    if (!status[replay]){
        switch(input){
            case 0x01:
                current_note = C;
                set_PWM(NOTES[current_note]);
                if(current_start == 0){ current_start = melody.time_length; }
                LED_PORT |= NOTES_LED;
            break;
            case 0x02:
                current_note = D;
                set_PWM(NOTES[current_note]);
                if(current_start == 0){ current_start = melody.time_length; }
                LED_PORT |= NOTES_LED;
            break;
            case 0x04:
                current_note = E;
                set_PWM(NOTES[current_note]);
                if(current_start == 0){ current_start = melody.time_length; }
                LED_PORT |= NOTES_LED;
            break;
            case 0x08:
                current_note = F;
                set_PWM(NOTES[current_note]);
                if(current_start == 0){ current_start = melody.time_length; }
                LED_PORT |= NOTES_LED;
            break;
            case 0x10:
                current_note = G;
                set_PWM(NOTES[current_note]);
                if(current_start == 0){ current_start = melody.time_length; }
                LED_PORT |= NOTES_LED;
            break;
            case 0x20:
                current_note = A;
                set_PWM(NOTES[current_note]);
                if(current_start == 0){ current_start = melody.time_length; }
                LED_PORT |= NOTES_LED;
            break;
            case 0x40:
                current_note = B;
                set_PWM(NOTES[current_note]);
                if(current_start == 0){ current_start = melody.time_length; }
                LED_PORT |= NOTES_LED;
            break;
            case 0x80:
                current_note = C1;
                set_PWM(NOTES[current_note]);
                if(current_start == 0){ current_start = melody.time_length; }
                LED_PORT |= NOTES_LED;
            break;
            default:
                LED_PORT &= ~NOTES_LED;
                if(status[recording] && (current_note != silent)){
                    current_duration = melody.time_length - current_start;
                    add_note(&melody, current_note, current_start, current_duration);
                }
                current_note = silent;
                set_PWM(NOTES[current_note]);
                current_start = 0;
                current_duration = 0;
            break;
        }
    }

    return state;
}

int updateMelodyLengthSM(int state){
    if (status[recording]){
        melody.time_length += PeriodGCD;
    }
    return state;
}

enum RecordingStates {Rstart, off, presson, on, pressoff };
int ToggleRecordingSM(int state){   
    unsigned char input = ~CTRL_PORT & RECORD_BUTTON;

    //Only record when not replaying
    if(!status[replay]){
        switch(state){
            case Rstart:
                state = off;
                break;
            case off:
                state = input ? presson : off;
                if(state == presson){
                    reset_melody(&melody); //starting a new melody.
                }
                break;
            case presson:
                state = input ? presson : on;
                break;
            case on:
                state = input ? pressoff : on;
                break;
            case pressoff:
                state = input ? pressoff : off;
                break;
            default:
                state = off;
                break;
        }

        switch(state){
            case presson:
                status[recording] = 1;
                LED_PORT |= RECORDING_LED;
                break;
            case pressoff:
                status[recording] = 0;
                LED_PORT &= ~RECORDING_LED;
            default:
                break;
        }
    }
    return state;
}

enum SendStates{Sstart, wait, send, hold };
int SendDataSM(int state){
    unsigned char input = ~CTRL_PORT & SEND_DATA_BUTTON;
    switch(state){
        case Sstart:
            state = wait;
            break;
        case wait:
            state = input ? send : wait;        
            break;
        case send:
            state = hold;
            status[send_data] = 1;
            LED_PORT |= SEND_LED;
            break;
        case hold:
            state = input ? hold : wait;
            LED_PORT |= SEND_LED;
            if(state == wait) { LED_PORT &= ~SEND_LED; }
            break;
        default:
            state = wait;
            break;
    }
    return state;
}

enum ReplayStates {RSstart, RSoff, RSpresson, RSon, RSpressoff };
int ToggleReplaySM(int state){   
    unsigned char input = ~CTRL_PORT & REPLAY_BUTTON;

    //Only replay if not recording
    if(!status[recording]){
        //If the melody ended go to the off state.
        if (state == RSon && status[replay] == 0){
            state = RSoff;
            LED_PORT &= ~REPLAY_LED;
        }

        switch(state){
            case RSstart:
                state = RSoff;
            break;
            case RSoff:
                state = input ? RSpresson : RSoff;
            break;
            case RSpresson:
                state = input ? RSpresson : RSon;
            break;
            case RSon:
                state = input ? RSpressoff : RSon;
            break;
            case RSpressoff:
                state = input ? RSpressoff : RSoff;
            break;
            default:
                state = RSoff;
            break;
        }

        switch(state){
            case RSpresson:
                status[replay] = 1;
                LED_PORT |= REPLAY_LED;
                break;
            case RSpressoff:
                status[replay] = 0;
                LED_PORT &= ~REPLAY_LED;
                break;
            default:
                break;
        }
    }
    return state;
}

int ReplayMelodySM(int state){
    static unsigned short melody_timer = 0;
    static unsigned char i = 0;
    static unsigned elapsedTime = 0;

    if(status[replay]){
        if (melody_timer < melody.time_length){
            if(melody_timer >= melody.times[i]){
                if(elapsedTime < melody.durations[i]){
                    set_PWM(NOTES[melody.notes[i]]);
                    elapsedTime += PeriodGCD;
                    LED_PORT |= NOTES_LED;
                } else {
                    set_PWM(NOTES[silent]);
                    LED_PORT &= ~NOTES_LED;
                    elapsedTime = 0;
                    i++;
                }
            } else { // time in between notes.
                set_PWM(NOTES[silent]);
            }
            melody_timer += PeriodGCD;
        } else { // End of melody, stop playing
            set_PWM(NOTES[silent]);
            status[replay] = 0;
        }
    } else {
        melody_timer = 0;
        i = 0;
        elapsedTime = 0;
    }
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
    tasks[i].Tick = &PlayNoteSM;
    i++;
    tasks[i].state = generic_start;
    tasks[i].period = PeriodGCD;
    tasks[i].elapsedTime = tasks[i].period;
    tasks[i].Tick = &updateMelodyLengthSM;
    i++;
    tasks[i].state = generic_start;
    tasks[i].period = PeriodGCD;
    tasks[i].elapsedTime = tasks[i].period;
    tasks[i].Tick = &ToggleRecordingSM;
    i++;
    tasks[i].state = generic_start;
    tasks[i].period = PeriodGCD;
    tasks[i].elapsedTime = tasks[i].period;
    tasks[i].Tick = &SendDataSM;
    i++;
    tasks[i].state = generic_start;
    tasks[i].period = PeriodGCD;
    tasks[i].elapsedTime = tasks[i].period;
    tasks[i].Tick = &ToggleReplaySM;
    i++;
    tasks[i].state = generic_start;
    tasks[i].period = PeriodGCD;
    tasks[i].elapsedTime = tasks[i].period;
    tasks[i].Tick = &ReplayMelodySM;
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