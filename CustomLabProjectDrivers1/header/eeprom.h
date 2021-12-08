#ifndef EEPROM_H
#define EEPROM_H
#include "melody.h"
#include <avr/eeprom.h>
#include <stdlib.h>

#define NUM_OFFSET  0
#define MELODY_OFFSET  1

typedef struct ProgramStatus{
    unsigned char num_melodies;
    const char * labels;
    Melody* melody;
}ProgramStatus;

void Program_init(ProgramStatus * program){
    program->melody = (Melody*)calloc(1, sizeof(Melody));
    reset_melody(program->melody);
    program->num_melodies = 0;
}

// i = 0 returns the first melody
void eeprom_load_melody(ProgramStatus * program, unsigned char i){
    eeprom_read_block(program->melody->serialized, ((i * SIZEOFMELODY) + 10), SIZEOFMELODY);
    deserialize_melody(program->melody, program->melody->serialized);
}

// //Always saves the melody at the ith position
void eeprom_save_melody(ProgramStatus * program){
    serialize_melody(program->melody, program->melody->serialized);
    eeprom_write_block(program->melody->serialized, ((program->num_melodies * SIZEOFMELODY) + 10), SIZEOFMELODY);
    program->num_melodies++;
}

#endif