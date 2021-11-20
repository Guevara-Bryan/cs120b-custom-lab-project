#ifndef TASK_H
#define TASK_H  

typedef struct taks {
    int state;
    unsigned long period;
    unsigned long elapsedTime;
    int (*Tick) (int);
} task;

#endif