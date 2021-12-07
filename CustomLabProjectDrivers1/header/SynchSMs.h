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
#define MELODY_LED 0x08



//============== SynchSMs setup ==============
task* tasks; // array of size numTasks
unsigned char numTasks = 8;
unsigned short PeriodGCD = 50;
void set_note(unsigned char note);
//============================================

//------------ Global Variables -------------
/*
    PCR - Process Control Register
    Allow you to set flags that let you control which processes the
    program can execute.

*/
unsigned char PCR = 0x00;
const unsigned char RECORDING = 0x01;
const unsigned char REPLAY = 0x02;
const unsigned char SEND_DATA = 0x04;
const unsigned char RECEIVE_DATA = 0x08;

Melody melody;
unsigned char current_note = silent;
unsigned short current_start = 0;
unsigned short current_duration = 0;
//-------------------------------------------

//-------------- Tick functions --------------
int Transmit(int state){
    if((PCR & SEND_DATA)){
        //disable data transmission until set again by another process.
        PCR &= ~SEND_DATA;

        serialize_melody(&melody, melody.serialized);

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
            memset(melody.serialized, 0, SIZEOFMELODY);

            USART_Transmit(TRANSMISSION_ACKNOWLEDGEMENT);
            for(unsigned short i = 0; i < SIZEOFMELODY; i++){
                melody.serialized[i] = USART_Receive();
            }
            melody.is_serialized = 1;
            deserialize_melody(&melody, &melody.serialized);
            if(melody.length > 0){
                    LED_PORT |= MELODY_LED;
                }else{
                    LED_PORT &= ~MELODY_LED;
                }
        }
    }
    return state;
}

int PlayNoteSM(int state){
    unsigned char input = ~NOTES_PORT;
    if (!(PCR & 0x0E)){
        switch(input){
            case 0x01:
                set_note(C);
            break;
            case 0x02:
                set_note(D);
            break;
            case 0x04:
                set_note(E);
            break;
            case 0x08:
                set_note(F);
            break;
            case 0x10:
                set_note(G);
            break;
            case 0x20:
                set_note(A);
            break;
            case 0x40:
                set_note(B);
            break;
            case 0x80:
                set_note(C1);
            break;
            default:
                LED_PORT &= ~NOTES_LED;
                set_PWM(NOTES[silent]);
                if((PCR & RECORDING) && (current_note != silent)){
                    current_duration = melody.time_length - current_start;
                    add_note(&melody, current_note, current_start, current_duration);
                    current_note = silent;
                    current_start = 0;
                    current_duration = 0;
                }
            break;
        }
    }

    return state;
}

int updateMelodyLengthSM(int state){
    if ((PCR == RECORDING)){
        melody.time_length += PeriodGCD;
    }
    return state;
}

enum RecordingStates {Rstart, off, presson, on, pressoff };
int ToggleRecordingSM(int state){   
    unsigned char input = ~CTRL_PORT & RECORD_BUTTON;

    //Only record when not replaying
    if(!(PCR & REPLAY)){
        switch(state){
            case Rstart:
                state = off;
                break;
            case off:
                state = input ? presson : off;
                if(state == presson){
                    reset_melody(&melody); //starting a new melody.
                    LED_PORT &= MELODY_LED;
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
                PCR |= RECORDING;
                LED_PORT |= RECORDING_LED;
                break;
            case pressoff:
                PCR &= ~RECORDING;
                LED_PORT &= ~RECORDING_LED;
                serialize_melody(&melody, &melody.serialized);
                if(melody.length > 0){
                    LED_PORT |= MELODY_LED;
                }else{
                    LED_PORT &= ~MELODY_LED;
                }
            default:
                break;
        }
    }
    return state;
}

enum SendStates{Sstart, wait, send, hold };
int SendDataSM(int state){
    unsigned char input = ~CTRL_PORT & SEND_DATA_BUTTON;
    // Only send data if no other process is active
    if(!PCR){
        switch(state){
            case Sstart:
                state = wait;
                break;
            case wait:
                state = input ? send : wait;        
                break;
            case send:
                state = hold;
                PCR |= SEND_DATA;
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
    }
    return state;
}

enum ReplayStates {RSstart, RSoff, RSpresson, RSon, RSpressoff };
int ToggleReplaySM(int state){   
    unsigned char input = ~CTRL_PORT & REPLAY_BUTTON;

    //Only replay if not recording
    if(!(PCR & RECORDING)){
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
                PCR |= REPLAY;
                LED_PORT |= REPLAY_LED;
                break;
            case RSpressoff:
                PCR &= ~REPLAY;
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

    if((PCR & REPLAY)){
        if (melody_timer < melody.time_length){ // If melody is not over

            if(melody_timer >= melody.times[i]){ // If a note was played at this time

                if(elapsedTime < melody.durations[i]){ // Play note for its duration
                    set_PWM(NOTES[melody.notes[i]]);
                    elapsedTime += PeriodGCD;
                    LED_PORT |= NOTES_LED;
                } else { // Silence while changing note
                    set_PWM(NOTES[silent]);
                    LED_PORT &= ~NOTES_LED;
                    elapsedTime = 0;
                    i++;
                }
            } else { // Silent in-between times.
                set_PWM(NOTES[silent]);
            }
            melody_timer += PeriodGCD;
        } else { // End of melody. Restart melody.
            set_PWM(NOTES[silent]);
            melody_timer = 0;
            i = 0;
            elapsedTime = 0;
        }


    } else { // Reset everything
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
    tasks[i].Tick = &Receive;
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

void set_note(unsigned char note){
    current_note = note;
    set_PWM(NOTES[current_note]);
    if (current_start == 0) { current_start = melody.time_length; }
    LED_PORT |= NOTES_LED;
}

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