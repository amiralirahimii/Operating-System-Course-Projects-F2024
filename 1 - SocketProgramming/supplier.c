#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/time.h>

#include "define.h"
#include "connection.c"

struct supplier{
    struct node my_node;
    struct message request;
    int haveRequest;
    int request_fd;
};

void read_tcp_message_supplier(struct supplier *my_supp, char* message, fd_set* master_set, int fd, int* max_sd){
    struct message msg = decode_message(message);
    if(msg.command==REQUEST_INGREDIENT){
        if(my_supp->haveRequest==1){
            send_tcp(fd, BUSY, EMPTY_STRING, EMPTY_NUMBER, EMPTY_STRING, EMPTY_NUMBER);
            close(fd);
            return;
        }
        if(fd>(*max_sd)){
            (*max_sd) = fd;
        }
        FD_SET(fd, master_set);
        my_supp->request.port = msg.port;
        my_supp->request_fd = fd;
        my_supp->haveRequest = 1;
        printf("%s \n", "new order");
        log_msg(my_supp->my_node.name, "new order arrived");
    }
}

void read_message_supplier(struct supplier *my_supp, char* message){
    struct message msg = decode_message(message);
    if(msg.command==SHOW_MESSAGE){
        write(1, msg.message, strlen(msg.message));
    }
    else if(msg.command==SHOW_SUPPLIERS){
        int fd = connect_tcp(msg.port);
        send_tcp(fd, SUPPLIER_DATA, my_supp->my_node.name, my_supp->my_node.tcp_port, EMPTY_STRING, EMPTY_NUMBER);
        close(fd);
    }
    else if(msg.command==CHECK_NAME){
        char* token = strtok(msg.message, "\n");
        if(strcmp(my_supp->my_node.name, token)==0){
            int fd = connect_tcp(msg.port);
            send_tcp(fd, THATS_MYNAME, EMPTY_STRING, EMPTY_NUMBER, EMPTY_STRING, EMPTY_NUMBER);
            close(fd);
        }
    }
}

void answerRequest(struct supplier *my_supp){
    if(my_supp->haveRequest){
        char buffer[1024] = {0};
        printf("your answer(yes/no)\n");
        memset(buffer, 0, 1024);
        read(0, buffer, 1024);
        char message[1024] = "";
        strcat(message, my_supp->my_node.name);
        strcat(message, " Supplier ");
        if(strcmp(buffer, "yes\n")==0){
            strcat(message, "accepted!");
            log_msg(my_supp->my_node.name, "order accepted");
            send_tcp(my_supp->request_fd, ACCEPT_CODE, message, EMPTY_NUMBER, EMPTY_STRING, EMPTY_NUMBER);
        }
        else if(strcmp(buffer, "no\n")==0){
            strcat(message, "denied!");
            log_msg(my_supp->my_node.name, "order rejected");
            send_tcp(my_supp->request_fd, REJECT_CODE, message, EMPTY_NUMBER, EMPTY_STRING, EMPTY_NUMBER);
        }
        my_supp->haveRequest = 0;
    }
    else{
        printf("No Requests\n");
    }
}

void read_command_supplier(struct supplier *my_supp, char* command){
    if(strcmp(command, "answer request\n")==0){
        answerRequest(my_supp);
    }
}


void process(struct supplier supp){
    int max_sd;
    fd_set master_set, working_set;
    FD_ZERO(&master_set);
    FD_SET(0, &master_set);
    FD_SET(supp.my_node.fd, &master_set);
    FD_SET(supp.my_node.tcp_fd, &master_set);
    if(supp.my_node.tcp_fd>supp.my_node.fd)
        max_sd = supp.my_node.tcp_fd;
    else
        max_sd = supp.my_node.fd;
    char buffer[1024] = {0};

    while (1) {
        working_set = master_set;
        select(max_sd + 1, &working_set, NULL, NULL, NULL);

        for (int i = 0; i <= max_sd; i++) {
            if (FD_ISSET(i, &working_set)) {
                if(i == 0){
                    memset(buffer, 0, 1024);
                    read(0, buffer, 1024);
                    read_command_supplier(&supp, buffer);
                }
                else if(i == supp.my_node.fd){
                    memset(buffer, 0, 1024);
                    recv(supp.my_node.fd, buffer, 1024, 0);
                    read_message_supplier(&supp, buffer);
                }
                else if(i == supp.my_node.tcp_fd){
                    memset(buffer, 0, 1024);
                    int fd = accept_tcp(supp.my_node.tcp_fd);
                    recv(fd, buffer, 1024, 0);
                    read_tcp_message_supplier(&supp, buffer, &master_set, fd, &max_sd);
                }
                else{
                    memset(buffer, 0, 1024);
                    int a = recv(i, buffer, 1024, 0);
                    if(a == 0){
                        supp.haveRequest = 0;
                        close(i);
                        FD_CLR(i, &master_set);
                    }
                }
            }
        }
    }
}

int main(int argc, char const *argv[]){
    int fd, port;
    if(argc != 2){
        //write(0, "Bad Argumant\n", strlen("Bad Argumant\n"));
        port = 8080;
    }
    else{
        port = atoi(argv[1]);
    }
    struct supplier my_supp;
    my_supp.my_node = setupNode(port, SUPPLIER);
    my_supp.haveRequest = 0;
    process(my_supp);
}