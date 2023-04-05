#ifndef DOOR_CONTROL_TASK_H
#define DOOR_CONTROL_TASK_H

#define DOOR_OPEN      1
#define DOOR_CLOSE     2
#define MAX_DOOR_MSG   64

typedef struct
{
    unsigned char data[MAX_DOOR_MSG];
    int len;
}door_msg_t;

void InitDoorTask();

#endif
