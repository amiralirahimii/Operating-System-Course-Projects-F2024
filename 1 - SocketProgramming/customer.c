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

typedef struct{
    char name[NAME_SIZE];
    int amount;
}Ingredient;

typedef struct {
    Ingredient* arr;
    int numOfIngs;
    int arrSize;
}IngredientsArray;

typedef struct
{
    char name[NAME_SIZE];
    IngredientsArray ings;
}Food;

typedef struct{
    Food* food;
    int numOfFoods;
}Foods;


struct customer{
    struct node my_node;
    Foods foods;
};

char* readFile(){    
    int fd = open("recipes.json", O_RDONLY);
    if (fd == -1) {
        perror("Failed to open file");
    }

    struct stat file_stat;
    if (fstat(fd, &file_stat) == -1) {
        perror("Failed to get file information");
        close(fd);
    }

    char* json_data = (char*)malloc(file_stat.st_size + 1);
    if (!json_data) {
        perror("Memory allocation failed");
        close(fd);
    }

    ssize_t read_size = read(fd, json_data, file_stat.st_size);
    close(fd);

    if (read_size == -1) {
        perror("Failed to read file");
        free(json_data);
    }

    json_data[read_size] = '\0';

    return json_data;
}

void extractFoodsFromJson(const char* json_data, Foods* foods) {
    foods->numOfFoods = 0;
    int arraySize = 10;
    foods->food = (Food*)malloc(arraySize * sizeof(Food));

    const char* ptr = json_data;

    while (1) {
        const char* foodStart = strchr(ptr, '"');
        if (foodStart == NULL) {
            break;
        }

        const char* foodEnd = strchr(foodStart + 1, '"');
        if (foodEnd == NULL) {
            break;
        }

        int foodNameLength = foodEnd - foodStart - 1;
        if (foodNameLength >= NAME_SIZE) {
            foodNameLength = NAME_SIZE - 1;
        }

        strncpy(foods->food[foods->numOfFoods].name, foodStart + 1, foodNameLength);
        foods->food[foods->numOfFoods].name[foodNameLength] = '\0';

        const char* ingStart = strchr(foodEnd, '{');
        if (ingStart == NULL) {
            break;
        }

        const char* ingEnd = strchr(ingStart, '}');
        if (ingEnd == NULL) {
            break;
        }

        IngredientsArray* ings = &foods->food[foods->numOfFoods].ings;
        ings->numOfIngs = 0;
        ings->arr = (Ingredient*)malloc(arraySize * sizeof(Ingredient));

        const char* ingPtr = ingStart;

        while (1) {
            const char* ingNameStart = strchr(ingPtr, '"');
            if (ingNameStart == NULL || ingNameStart > ingEnd) {
                break;
            }

            const char* ingNameEnd = strchr(ingNameStart + 1, '"');
            if (ingNameEnd == NULL || ingNameEnd > ingEnd) {
                break;
            }

            int ingNameLength = ingNameEnd - ingNameStart - 1;
            if (ingNameLength >= NAME_SIZE) {
                ingNameLength = NAME_SIZE - 1;
            }

            strncpy(ings->arr[ings->numOfIngs].name, ingNameStart + 1, ingNameLength);
            ings->arr[ings->numOfIngs].name[ingNameLength] = '\0';

            const char* colon = strchr(ingNameEnd, ':');
            if (colon == NULL || colon > ingEnd) {
                break;
            }

            const char* comma = strchr(colon, ',');
            if (comma == NULL || comma > ingEnd) {
                comma = ingEnd;
            }

            ings->arr[ings->numOfIngs].amount = atoi(colon + 1);

            ingPtr = comma + 1;
            ings->numOfIngs++;

            if (ings->numOfIngs == arraySize) {
                arraySize *= 2;
                ings->arr = (Ingredient*)realloc(ings->arr, arraySize * sizeof(Ingredient));
            }
        }

        foods->numOfFoods++;

        if (foods->numOfFoods == arraySize) {
            arraySize *= 2;
            foods->food = (Food*)realloc(foods->food, arraySize * sizeof(Food));
        }

        ptr = ingEnd + 1;
    }
}



void getFoods(Foods *foods){
    char* json_data=readFile();
    if (!strcmp(json_data,"failed"))   return;
    extractFoodsFromJson(json_data,foods);
    free(json_data);
    // for (int i = 0; i < foods->numOfFoods; i++) {
    //     printf("%d- %s\n", i+1,foods->food[i].name);
    //     for (int j = 0; j < foods->food[i].ings.numOfIngs; j++) {
    //         printf("        %s : %d\n", foods->food[i].ings.arr[j].name, foods->food[i].ings.arr[j].amount);
    //     }
    // }
}

void read_message_customer(struct customer *my_cust, char* message){
    struct message msg = decode_message(message);
    if(msg.command==SHOW_MESSAGE){
        write(1, msg.message, strlen(msg.message));
    }
    else if(msg.command==RESTURANT_DATA){
        char print[1024] = "";
        char itoa[10] = "";
        strcat(print, msg.message);
        strcat(print, " ");
        sprintf(itoa, "%d", msg.port);
        strcat(print, itoa);
        strcat(print, "\n");
        write(1, print, strlen(print));
    }
    else if(msg.command==ACCEPT_CODE || msg.command==REJECT_CODE){
        printf("%s\n", msg.message);
        log_msg(my_cust->my_node.name, msg.message);
    }
    else if(msg.command==CHECK_NAME){
        char* token = strtok(msg.message, "\n");
        if(strcmp(my_cust->my_node.name, token)==0){
            int fd = connect_tcp(msg.port);
            send_tcp(fd, THATS_MYNAME, EMPTY_STRING, EMPTY_NUMBER, EMPTY_STRING, EMPTY_NUMBER);
            close(fd);
        }
    }
}

void alarm_handler(int sig){
    printf("Time out \n");
}

void requestFood(struct customer *my_cust){
    char buffer[1024] = {0};
    printf("name of food: \n");
    memset(buffer, 0, 1024);
    read(0, buffer, 1024);
    char name[NAME_SIZE];
    strcpy(name, buffer);
    

    printf("port of resturant: \n");
    memset(buffer, 0, 1024);
    read(0, buffer, 1024);
    int port = atoi(buffer);

    printf("waiting for resturant response... \n");

    int fd = connect_tcp(port);
    if(fd == EMPTY_NUMBER){
        log_msg(my_cust->my_node.name, "Entered Wrong Port");
        printf("Wrong Port\n");
        return;
    }

    signal(SIGALRM, alarm_handler);
    siginterrupt(SIGALRM, 1);

    alarm(120);
    
    send_tcp(fd, ORDER_FOOD, my_cust->my_node.name, my_cust->my_node.tcp_port, name, EMPTY_NUMBER);
    memset(buffer, 0, 1024);
    recv(fd, buffer, 1024, 0);
    alarm(0);

    read_message_customer(my_cust, buffer);
    close(fd);
}

void showMenu(struct customer cust){
    for(int i=0 ; i<cust.foods.numOfFoods ; i++){
        printf("%d-%s\n", i+1, cust.foods.food[i].name);
    }
}

void read_command_customer(struct customer *my_cust, char* command){
    if(strcmp(command, "show restaurants\n")==0){
        send_broadcast(my_cust->my_node, SHOW_RESTURANTS, EMPTY_STRING, my_cust->my_node.tcp_port, EMPTY_STRING, EMPTY_NUMBER);
    }
    else if(strcmp(command, "order food\n")==0){
        requestFood(my_cust);
    }
    else if(strcmp(command, "show menu\n")==0){
        showMenu(*my_cust);
    }
}

void process(struct customer cust){

    int max_sd;
    fd_set master_set, working_set;
    FD_ZERO(&master_set);
    FD_SET(0, &master_set);
    FD_SET(cust.my_node.fd, &master_set);
    FD_SET(cust.my_node.tcp_fd, &master_set);
    if(cust.my_node.tcp_fd>cust.my_node.fd)
        max_sd = cust.my_node.tcp_fd;
    else
        max_sd = cust.my_node.fd;
    char buffer[1024] = {0};

    while (1) {
        working_set = master_set;
        select(max_sd + 1, &working_set, NULL, NULL, NULL);

        for (int i = 0; i <= max_sd; i++) {
            if (FD_ISSET(i, &working_set)) {
                if(i == 0){
                    memset(buffer, 0, 1024);
                    read(0, buffer, 1024);
                    read_command_customer(&cust, buffer);
                }
                else if(i == cust.my_node.fd){
                    memset(buffer, 0, 1024);
                    recv(cust.my_node.fd, buffer, 1024, 0);
                    read_message_customer(&cust, buffer);
                }
                else if(i == cust.my_node.tcp_fd){
                    memset(buffer, 0, 1024);
                    int fd = accept_tcp(cust.my_node.tcp_fd);
                    
                    recv(fd, buffer, 1024, 0);
                    read_message_customer(&cust, buffer);
                    close(fd);
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
    Foods foods;
    getFoods(&foods);
    struct customer my_cust;
    my_cust.my_node = setupNode(port, CUSTOMER);
    my_cust.foods = foods;
    process(my_cust);
}