#ifndef MELODY_H
#define MELODY_H
#define MAX_NOTES 100 // Maximum notes per melody.
enum Notes {silent, C, D, E, F, G, A, B, C1};
const float NOTES[9] = {0.00, 261.63, 293.66, 329.63, 349.23, 392.00, 440.00, 493.00, 523.25};

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
}Melody;


void reset_melody(Melody * _melody){
    for(unsigned char i = 0; i < _melody->length; i++){
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
#endif