#ifndef NODE_H
#define NODE_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/time.h>

#include "define.h"
#include "messageHandler.c"
#include "logger.c"


struct node{
    char* name;
    int port;
    int fd;
    struct sockaddr_in bc_address;
    int tcp_fd;
    int tcp_port;
};

void signup(int fd, struct sockaddr_in bc_address, char* name, int role, int tcp_port, int tcp_fd);

int checkName(int fd, struct sockaddr_in bc_address, char* name, int role, int tcp_port, int tcp_fd);

int generate_port();

struct node setupNode(int port, int role);


#endif