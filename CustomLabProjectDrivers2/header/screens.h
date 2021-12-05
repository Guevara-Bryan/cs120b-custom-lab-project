#ifndef SCREENS_H
#define SCREENS_H
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "cursor.h"

#define SCREEN_LENGTH 29

typedef struct Screen{
    char content[2][SCREEN_LENGTH];
    unsigned char frame;
    Cursor cursor_pointer;
} Screen;

void Screen_init(Screen * _screen, unsigned char frame, const char *data){
    _screen->frame = frame;
    strcpy(_screen->content[_screen->frame], data);
    cursor_init(&_screen->cursor_pointer, 1, 15);
}

void update_char(Screen* _screen, unsigned char pos, unsigned char character){
    if(pos < SCREEN_LENGTH && pos >= 0){
        _screen->content[0][pos] = character;
    }
}

//==========================================================================================

typedef struct Menu{
    Screen* screens;
    unsigned char size;
    unsigned char current_screen;
} Menu;


void Menu_init(Menu* menu, char scrn_number, char current){
    menu->size = scrn_number;
    menu->screens = (Screen*)calloc(menu->size, sizeof(Screen));
    if (current < scrn_number){
        menu->current_screen = current;
    }
}

// Circular next Screen
void nextScreen(Menu* menu){
    menu->current_screen = (menu->current_screen + 1) % menu->size;
}
// Circular previous screen
void previousScreen(Menu* menu){
    menu->current_screen = menu->current_screen - 1 >= 0 ? menu->current_screen - 1 : menu->size - 1;
}

//circular scroll down
void scroll_down(Menu* menu, unsigned char screen){
    menu->screens[screen].frame = (menu->screens[screen].frame + 1) % 2;
}

//circular scroll up
void scroll_up(Menu* menu, unsigned char screen){
    menu->screens[screen].frame = (menu->screens[screen].frame == 1) ? 0 : menu->screens[screen].frame + 1;
}

Screen * getScreen(Menu* menu, unsigned char pos){
    if(pos >= 0 && pos < menu->size){
        return &menu->screens[pos];
    }
    return 0;
}

void update_screen(Menu* menu, unsigned char screen, unsigned char frame, char * data){
    strcpy(menu->screens[screen].content[frame], data);
}

// Returns the content of the current screen
char * getCurrentScreenContent(Menu* menu, unsigned char frame){
    return menu->screens[menu->current_screen].content[frame];
}
#endif