#ifndef MELODY_H
#define MELODY_H
#define MAX_NOTES 100 // Maximum notes per melody.
#define SIZEOFMELODY sizeof(char) + sizeof(short) + (MAX_NOTES * sizeof(char)) + 2 * (MAX_NOTES * sizeof(short))
#define NOTES_OFFSET 3
#define TIMES_OFFSET 103
#define DURATIONS_OFFSET 303

#include <string.h>
#include <stdio.h>
#include <stdlib.h>


enum Notes {silent, C, D, E, F, G, A, B, C1};
const float NOTES[9] = {0.00, 261.63, 293.66, 329.63, 349.23, 392.00, 440.00, 493.00, 523.25};
const unsigned char* char_notes = "SCDEFGAB*";

typedef struct Melody{
    // How many notes in the melody
    unsigned char length;
    // How long the melody lasts in millisenconds
    unsigned short time_length;
    // buffer with indexes of the notes
    unsigned char notes[MAX_NOTES];
    // buffer with times when note starts playing
    unsigned short times[MAX_NOTES];
    // buffer with duration of each note
    unsigned short durations[MAX_NOTES];
    // Contains the Melody in serialized form.
    char serialized[SIZEOFMELODY];
    // Tells wether the melody has been serialized or not.
    unsigned char is_serialized;
}Melody;


void reset_melody(Melody * _melody){
    for(unsigned char i = 0; i < MAX_NOTES; i++){
        _melody->notes[i] = 0;
        _melody->times[i] = 0;
        _melody->durations[i] = 0;
    }

    _melody->length = 0;
    _melody->time_length = 0;
}

void add_note(Melody * _melody, int note, unsigned short start, unsigned short dur){
    if(_melody->length < MAX_NOTES){
        _melody->notes[_melody->length] = note;
        _melody->times[_melody->length] = start;
        _melody->durations[_melody->length] = dur;
        _melody->length++;
    }
}

// Takes the melody and serializes it into the "data" array
void serialize_melody(Melody* melody, char* data){

    memset(data, 0, SIZEOFMELODY);

    memcpy(&data[NOTES_OFFSET], melody->notes, MAX_NOTES);
    
    unsigned short number;
    number = melody->time_length;
    data[0] = melody->length;
    data[1] = (char)number;
    data[2] = (char)(number >> 8);
    for (int i = 1; i < (MAX_NOTES * sizeof(short)); i += sizeof(short)){
        number = melody->times[i];
        data[TIMES_OFFSET + i - 1] = (char)number;
        data[TIMES_OFFSET + i] = (char)(number >> 8);


        number = melody->durations[i];
        data[DURATIONS_OFFSET + i - 1] = (char)number;
        data[DURATIONS_OFFSET + i] = (char)(number >> 8);
    }
    melody->is_serialized = 1;
}

// Takes the serial data in "data" array and stores it in the melody.
void deserialize_melody(Melody* melody, const char* data){
    reset_melody(melody);

    memcpy(melody->notes, &data[NOTES_OFFSET], MAX_NOTES);
    melody->length = data[0];
    melody->time_length = (short)data[2];
    melody->time_length <<= 8;
    melody->time_length |= (short)data[1];

    for(int i = 1; i < (MAX_NOTES * sizeof(short)); i += sizeof(short)){
        melody->times[i] = (short)data[TIMES_OFFSET + i];
        melody->times[i] <<= 8;
        melody->times[i] |= (short)data[TIMES_OFFSET + i - 1];

        // set the hight byte
        melody->durations[i] = data[DURATIONS_OFFSET + i];
        melody->durations[i] <<= 8;
        melody->durations[i - 1] |= data[DURATIONS_OFFSET + i - 1];
    }
}
#endif