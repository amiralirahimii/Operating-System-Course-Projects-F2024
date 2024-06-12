#ifndef CONNECTION_H
#define CONNECTION_H

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
// #include "messageHandler.h"
#include "node.c"

int connect_tcp(int port);

int accept_tcp(int server_fd);

void send_broadcast(struct node my_node ,int command, char* message, int port, char* itemname, int itemnumber);

void send_tcp(int tcp_fd ,int command, char* message, int port, char* itemname, int itemnumber);

#endif