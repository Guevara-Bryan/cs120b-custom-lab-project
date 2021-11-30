#ifndef SCREENS_H
#define SCREENS_H
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define SCREEN_LENGTH 29

typedef struct Screen{
    char content[SCREEN_LENGTH];
    unsigned char cursor_pos;
} Screen;

void Screen_init(Screen * _screen, unsigned char _cursor, const char *data){
    strcpy(_screen->content, data);
    _screen->cursor_pos = _cursor;
}

void update_char(Screen* _screen, unsigned char pos, unsigned char character){
    if(pos < SCREEN_LENGTH && pos >= 0){
        _screen->content[pos] = character;
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

Screen * getScreen(Menu* menu, unsigned char pos){
    if(pos >= 0 && pos < menu->size){
        return &menu->screens[pos];
    }
    return 0;
}

void update_screen(Menu* menu, unsigned char screen, char * data){
    strcpy(menu->screens[screen].content, data);
}

// Returns the content of the current screen
char * getCurrentScreenContent(Menu* menu){
    return menu->screens[menu->current_screen].content;
}
#endif