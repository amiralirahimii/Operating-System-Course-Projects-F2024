#include "node.h"

void alarm_handler2(int sig){

}

int checkName(int fd, struct sockaddr_in bc_address, char* name, int role, int tcp_port, int tcp_fd){
    //check name already in use
    char coded[1024] = "";
    struct message msg;
    init_msg(&msg, CHECK_NAME, name, tcp_port, EMPTY_STRING, EMPTY_NUMBER);
    code_message(msg, coded);
    sendto(fd, coded, strlen(coded), 0,(struct sockaddr *)&bc_address, sizeof(bc_address));
    char buffer[1024] = {0};

    signal(SIGALRM, alarm_handler2);
    siginterrupt(SIGALRM, 1);

    memset(buffer, 0, 1024);
    alarm(1);
    int new_fd;
    struct sockaddr_in client_address;
    int address_len = sizeof(client_address);
    new_fd = accept(tcp_fd, (struct sockaddr *)&client_address, (socklen_t*) &address_len);

    alarm(0);
    if(new_fd == -1){
        return 0;
    }

    recv(new_fd, buffer, 1024, 0);
    msg = decode_message(buffer);
    if(msg.command==THATS_MYNAME){
        printf("Name Already In Use\n");
        return 1;
    }
}

void signup(int fd, struct sockaddr_in bc_address, char* name, int role, int tcp_port, int tcp_fd){
    char* message = "Please Enter you username: ";
    write(1, message, strlen(message));
    read(0, name, 1024);

    while(checkName(fd, bc_address, name, role, tcp_port, tcp_fd) == 1){
        write(1, message, strlen(message));
        read(0, name, 1024);
    }

    char welcome[1024] = "";
    char* token = strtok(name, "\n");

    if(token!=NULL){
        strcat(welcome, "welcome ");
        strcat(welcome, name);
        if(role == RESTURANT)
            strcat(welcome, " as resturant!!\n");
        else if(role == SUPPLIER)
            strcat(welcome, " as supplier!!\n");
        else if(role == CUSTOMER)
            strcat(welcome, " as customer!!\n");
        else
            strcat(welcome, " wtf!!");
    }
    else{
        strcat(welcome, "ridi");
    }

    creating_log_file(token);
    log_msg(token, "User Entered");

    char coded[1024] = "";
    struct message msg;
    memset(coded, 0, 1024);
    init_msg(&msg, SHOW_MESSAGE, welcome, EMPTY_NUMBER, EMPTY_STRING, EMPTY_NUMBER);
    code_message(msg, coded);
    sendto(fd, coded, strlen(coded), 0,(struct sockaddr *)&bc_address, sizeof(bc_address));
}

int generate_port(){
    int lower = 49152, upper = 65535;
    int randomNum;

    srand(time(0));

    randomNum = (rand() % (upper - lower + 1)) + lower;

    return randomNum;
}

struct node setupNode(int port, int role){
    //setup udp
    int fd, broadcast = 1, opt = 1;
    struct sockaddr_in bc_address;

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    setsockopt(fd, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));
    setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));

    bc_address.sin_family = AF_INET; 
    bc_address.sin_port = htons(port);
    bc_address.sin_addr.s_addr = inet_addr("255.255.255.255");

    bind(fd, (struct sockaddr *)&bc_address, sizeof(bc_address));
    //setup tcp
    struct sockaddr_in tcp_address;
    int tcp_fd;
    tcp_fd = socket(AF_INET, SOCK_STREAM, 0);

    setsockopt(tcp_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    tcp_address.sin_family = AF_INET;
    tcp_address.sin_addr.s_addr = INADDR_ANY;

    int tcp_port;
    int found_port = 0;
    while(!found_port){
        tcp_port = generate_port();
        tcp_address.sin_port = htons(tcp_port);
        if(bind(tcp_fd, (struct sockaddr *)&tcp_address, sizeof(tcp_address)) != -1){
            found_port = 1;
        }
    }
    
    listen(tcp_fd, 4);

    char* name = malloc(20*sizeof(char));
    signup(fd, bc_address, name, role, tcp_port, tcp_fd);
    struct node result = {name, port, fd, bc_address, tcp_fd, tcp_port};

    return result;
}