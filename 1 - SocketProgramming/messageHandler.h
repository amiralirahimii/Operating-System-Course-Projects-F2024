#ifndef MESSAGE_HANDLER_H
#define MESSAGE_HANDLER_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "define.h"

struct message{
    int command;
    char message[100];
    int port;
    char itemname[20];
    int itemnumber;
};

void code_message(struct message msg, char* coded);

struct message decode_message(char*);


#endif