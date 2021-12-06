#ifndef MELODY_H
#define MELODY_H
#define MAX_NOTES 100 // Maximum notes per melody.
#define SIZEOFMELODY sizeof(char) + sizeof(short) + (MAX_NOTES * sizeof(char)) + 2 * (MAX_NOTES * sizeof(short))
#define NOTES_OFFSET 3
#define TIMES_OFFSET 103
#define DURATIONS_OFFSET 303

#include <string.h>

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

union SandC
{
    unsigned short s;
    unsigned char bytes[2];
} temp;

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

    data[0] = melody->length;

    temp.s = melody->time_length;
    data[1] = temp.bytes[0];
    data[2] = temp.bytes[1];
    memcpy(&data[NOTES_OFFSET], melody->notes, MAX_NOTES);
    int j = 0;
    for(int i = 0; i < MAX_NOTES; i++){
        temp.s = melody->times[i];
        j = sizeof(short) * i;
        data[TIMES_OFFSET + j] = temp.bytes[0];
        data[TIMES_OFFSET + j + 1] = temp.bytes[1];

        temp.s = melody->durations[i];
        data[DURATIONS_OFFSET + j] = temp.bytes[0];
        data[DURATIONS_OFFSET + j + 1] = temp.bytes[1];
    }
    melody->is_serialized = 1;
}

// Takes the serial data in "data" array and stores it in the melody.
void deserialize_melody(Melody* melody, const char* data){
    reset_melody(melody);

    memcpy(melody->notes, &data[NOTES_OFFSET], MAX_NOTES);

    melody->length = data[0];
    temp.bytes[1] = data[2];
    temp.bytes[0] = data[1];
    melody->time_length = temp.s;
    int j = 0;
    for(int i = 0; i < MAX_NOTES; i++){
        j = sizeof(short) * i;
        temp.bytes[0] = data[TIMES_OFFSET + j + 1];
        temp.bytes[1] = data[TIMES_OFFSET + j];
        melody->times[i] = temp.s;

        temp.bytes[0] = data[DURATIONS_OFFSET + j + 1];
        temp.bytes[1] = data[DURATIONS_OFFSET + j];
        melody->durations[i] = temp.s;
    }
}
#endif