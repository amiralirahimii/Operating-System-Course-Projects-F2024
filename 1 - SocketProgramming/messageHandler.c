#include "messageHandler.h"

void init_msg(struct message* msg, int command, char* message, int port, char* itemname, int itemnumber){
    msg->command = command;
    strcpy(msg->message, message);
    msg->port = port;
    strcpy(msg->itemname, itemname);
    msg->itemnumber = itemnumber;
}

void code_message(struct message msg, char* coded){
    memset(coded, 0, strlen(coded));
    char itoa[10] = "";
    sprintf(itoa, "%d", msg.command);
    strcat(coded, itoa);
    strcat(coded, PARSER);
    strcat(coded, msg.message);
    strcat(coded, PARSER);
    sprintf(itoa, "%d", msg.port);
    strcat(coded, itoa);
    strcat(coded, PARSER);
    strcat(coded, msg.itemname);
    strcat(coded, PARSER);
    sprintf(itoa, "%d", msg.itemnumber);
    strcat(coded, itoa);
}

struct message decode_message(char* msg){
    struct message result;

    char* token = strtok(msg, PARSER);
    if(token != NULL) {
        result.command = atoi(token);
        token = strtok(NULL, PARSER);
    }
    if(token != NULL) {
        strcpy(result.message, token);
        token = strtok(NULL, PARSER);
    }
    if(token != NULL) {
        result.port = atoi(token);
        token = strtok(NULL, PARSER);
    }
    if(token != NULL) {
        strcpy(result.itemname, token);
        token = strtok(NULL, PARSER);
    }
    if(token != NULL) {
        result.itemnumber = atoi(token);
        token = strtok(NULL, PARSER);
    }
    return result;
}
