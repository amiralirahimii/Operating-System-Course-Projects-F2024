#include "connection.h"

int connect_tcp(int port){
    int fd;
    struct sockaddr_in server_address;
    
    fd = socket(AF_INET, SOCK_STREAM, 0);
    
    server_address.sin_family = AF_INET; 
    server_address.sin_port = htons(port); 
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) { // checking for errors
        return EMPTY_NUMBER;
    }

    return fd;
}

int accept_tcp(int server_fd){
    int client_fd;
    struct sockaddr_in client_address;
    int address_len = sizeof(client_address);
    
    client_fd = accept(server_fd, (struct sockaddr *)&client_address, (socklen_t*) &address_len);
    // write(1, "connected", strlen("connected"));

    return client_fd;
}

void send_broadcast(struct node my_node ,int command, char* message, int port, char* itemname, int itemnumber){
    //use UDP
    char coded[1024] = "";
    struct message msg;
    init_msg(&msg, command, message, port, itemname, itemnumber);
    code_message(msg, coded);
    sendto(my_node.fd, coded, strlen(coded), 0,(struct sockaddr *)&my_node.bc_address, sizeof(my_node.bc_address));
}

void send_tcp(int tcp_fd ,int command, char* message, int port, char* itemname, int itemnumber){
    //use TCP
    char coded[1024] = "";
    struct message msg;
    init_msg(&msg, command, message, port, itemname, itemnumber);
    code_message(msg, coded);
    send(tcp_fd, coded, strlen(coded), 0);
}