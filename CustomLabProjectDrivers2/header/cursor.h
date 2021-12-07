#ifndef CURSOR_H
#define CURSOR_H

#define screen_width 16
#define screen_height 2

//Note top-left is (0,0)
//Bottom-right is (1, 15)
typedef struct Cursor{
    unsigned char x;
    unsigned char y;
    unsigned char linear; // position of the cursor in a linear array.
}Cursor;

void cursor_init(Cursor * cursor, unsigned char x, unsigned char y){
    if (x < screen_width && x >= 0){
        cursor->x = x;
    }
    if (y < screen_height >= 0){
        cursor->y = y;
    }

    cursor->linear = ((cursor->y * screen_width) + 1) + cursor->x;
}

void move_up(Cursor * cursor){
    cursor->y = 0;

    cursor->linear = ((cursor->y * screen_width) + 1) + cursor->x;
}

void move_down(Cursor * cursor){
    cursor->y = 1;
    cursor->linear = ((cursor->y * screen_width) + 1) + cursor->x;
}

void move_left(Cursor * cursor){
    cursor->x = cursor->x - 1 >= 0 ? cursor->x - 1 : cursor->x;
    cursor->linear = ((cursor->y * screen_width) + 1) + cursor->x;
}

void move_right(Cursor * cursor){
    cursor->x = cursor->x + 1 < screen_width ? cursor->x + 1 : cursor->x; 
    cursor->linear = ((cursor->y * screen_width) + 1) + cursor->x;
}



#endif