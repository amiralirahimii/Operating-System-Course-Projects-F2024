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

struct order{
    char name[NAME_SIZE];
    int port;
    char food_name[NAME_SIZE];
    int status;
    int fd;
};

struct resturant{
    struct node my_node;
    int working;
    IngredientsArray ingredients;
    Foods foods;
    struct order* orders;
    int orders_size;
};

int read_message_resturant(struct resturant*, char*);

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


void extractIngredients(const Foods* foods, IngredientsArray* ingredients) {
    if (ingredients->arr == NULL) {
        ingredients->arrSize = 10;
        ingredients->numOfIngs = 0;
        ingredients->arr = (Ingredient*)malloc(ingredients->arrSize * sizeof(Ingredient));
    }

    for (int i = 0; i < foods->numOfFoods; i++) {
        const Food* currentFood = &foods->food[i];
        for (int j = 0; j < currentFood->ings.numOfIngs; j++) {
            const Ingredient* currentIngredient = &currentFood->ings.arr[j];
            int isDuplicate = 0;
            for (int k = 0; k < ingredients->numOfIngs; k++) {
                if (strcmp(currentIngredient->name, ingredients->arr[k].name) == 0) {
                    isDuplicate = 1;
                    break;
                }
            }
            if (!isDuplicate) {
                if (ingredients->numOfIngs == ingredients->arrSize) {
                    ingredients->arrSize *= 2;
                    ingredients->arr = (Ingredient*)realloc(ingredients->arr, ingredients->arrSize * sizeof(Ingredient));
                }
                strcpy(ingredients->arr[ingredients->numOfIngs].name, currentIngredient->name);
                ingredients->arr[ingredients->numOfIngs].amount = 0;
                ingredients->numOfIngs++;
            }
        }
    }
    // printf("All Ingredients:\n");
    // for (int i = 0; i < ingredients->numOfIngs; i++) {
    //     printf("Ingredient: %s\n", ingredients->arr[i].name);
    // }
}



void printIngredients(struct resturant rest){
    for(int i=0 ; i<rest.ingredients.numOfIngs ; i++){
        printf("   %s : %d\n", rest.ingredients.arr[i].name, rest.ingredients.arr[i].amount);
    }
}

void printFoods(struct resturant rest){
    for(int i=0 ; i<rest.foods.numOfFoods ; i++){
        printf("%d-%s\n", i+1, rest.foods.food[i].name);
        for(int j=0 ; j<rest.foods.food[i].ings.numOfIngs ; j++){
            printf("     %s : %d\n", rest.foods.food[i].ings.arr[j].name, rest.foods.food[i].ings.arr[j].amount);
        }
    }
}

void addIngr(struct resturant *rest, char* name, int amount){
    for(int i=0 ; i<rest->ingredients.numOfIngs ; i++){
        char withEnterName[NAME_SIZE] = {0};
        strcat(withEnterName, rest->ingredients.arr[i].name);
        strcat(withEnterName, "\n");
        if(strcmp(name, withEnterName)==0){
            rest->ingredients.arr[i].amount += amount;
        }
    }
}

void alarm_handler(int sig){
    printf("Time out \n");
}

void requestIngr(struct resturant *rest){
    char buffer[1024] = {0};
    printf("port of supplier: \n");
    memset(buffer, 0, 1024);
    read(0, buffer, 1024);
    int port = atoi(buffer);

    printf("name of ingredient: \n");
    memset(buffer, 0, 1024);
    read(0, buffer, 1024);
    char name[NAME_SIZE];
    strcpy(name, buffer);

    printf("number of ingredient: \n");
    memset(buffer, 0, 1024);
    read(0, buffer, 1024);
    int amount = atoi(buffer);

    printf("waiting for supplier response... \n");

    int fd = connect_tcp(port);
    if(fd == EMPTY_NUMBER){
        printf("Wrong Port\n");
        return;
    }

    signal(SIGALRM, alarm_handler);
    siginterrupt(SIGALRM, 1);

    alarm(90);
    
    send_tcp(fd, REQUEST_INGREDIENT, EMPTY_STRING, rest->my_node.tcp_port, name, amount);
    memset(buffer, 0, 1024);
    recv(fd, buffer, 1024, 0);
    alarm(0);

    if(read_message_resturant(rest, buffer) == ACCEPT_CODE){
        log_msg(rest->my_node.name, "your order frome supplier accepted");
        addIngr(rest, name, amount);
    }
    close(fd);
}

void addOrder(struct resturant *my_rest, struct message msg, int fd){
    char* token = strtok(msg.itemname, "\n");
    strcpy(my_rest->orders[my_rest->orders_size].food_name, token);
    my_rest->orders[my_rest->orders_size].fd = fd;
    strcpy(my_rest->orders[my_rest->orders_size].name, msg.message);
    my_rest->orders[my_rest->orders_size].port = msg.port;
    my_rest->orders[my_rest->orders_size].status = AWAKE;
    my_rest->orders_size++;
}


void read_tcp_message_resturant(struct resturant *my_rest, char* message, fd_set* master_set, int fd, int* max_sd){
    struct message msg = decode_message(message);
    if(msg.command==SUPPLIER_DATA){
        char print[1024] = "";
        char itoa[10] = "";
        strcat(print, msg.message);
        strcat(print, " ");
        sprintf(itoa, "%d", msg.port);
        strcat(print, itoa);
        strcat(print, "\n");
        write(1, print, strlen(print));
        close(fd);
    }
    else if(msg.command==ORDER_FOOD){
        if(fd>(*max_sd)){
            (*max_sd) = fd;
        }
        FD_SET(fd, master_set);
        addOrder(my_rest, msg, fd);
        printf("%s \n", "new order");
        log_msg(my_rest->my_node.name, "new order recieved");
    }
}


int read_message_resturant(struct resturant *my_rest, char* message){
    struct message msg = decode_message(message);
    if(msg.command==SHOW_MESSAGE){
        write(1, msg.message, strlen(msg.message));
    }
    
    else if(msg.command==SHOW_RESTURANTS){
        int fd = connect_tcp(msg.port);
        send_tcp(fd, RESTURANT_DATA, my_rest->my_node.name, my_rest->my_node.tcp_port, EMPTY_STRING, EMPTY_NUMBER);
        close(fd);
    }
    else if(msg.command==ACCEPT_CODE || msg.command==REJECT_CODE){
        printf("%s\n", msg.message);
        return msg.command;
    }
    else if(msg.command==BUSY){
        printf("%s\n", "supplier is busy");
    }
    else if(msg.command==CHECK_NAME){
        char* token = strtok(msg.message, "\n");
        if(strcmp(my_rest->my_node.name, token)==0){
            int fd = connect_tcp(msg.port);
            send_tcp(fd, THATS_MYNAME, EMPTY_STRING, EMPTY_NUMBER, EMPTY_STRING, EMPTY_NUMBER);
            close(fd);
        }
    }
    return EMPTY_NUMBER;
}

void showRequests(struct resturant* rest){
    for(int i=0; i<rest->orders_size; i++){
        if(rest->orders[i].status == AWAKE){
            char print[1024] = "";
            char itoa[10] = "";
            strcat(print, rest->orders[i].name);
            strcat(print, " ");
            sprintf(itoa, "%d", rest->orders[i].port);
            strcat(print, itoa);
            strcat(print, " ");
            strcat(print, rest->orders[i].food_name);
            strcat(print, "\n");
            write(1, print, strlen(print));
        }
    }
}

int tryCook(struct resturant *rest, int index){
    int findFoodIdx = EMPTY_NUMBER;
    for(int i=0; i<rest->foods.numOfFoods; i++){
        if(strcmp(rest->foods.food[i].name, rest->orders[index].food_name)==0){
            findFoodIdx = i;
        }
    }

    if(findFoodIdx == EMPTY_NUMBER){
        printf("Food doesnt exist in menu\n");
        log_msg(rest->my_node.name, "wanted to cook but Food doesnt exist in menu");
        return 0;
    }

    int haveThem = 1;
    for(int i=0; i<rest->foods.food[findFoodIdx].ings.numOfIngs;i++){
        for(int j=0;j<rest->ingredients.arrSize;j++){
            if(strcmp(rest->foods.food[findFoodIdx].ings.arr[i].name, rest->ingredients.arr[j].name)==0){
                if(rest->foods.food[findFoodIdx].ings.arr[i].amount > rest->ingredients.arr[j].amount){
                    haveThem = 0;
                }
            }
        }
    }

    if(!haveThem){
        printf("Ingredients not enough\n");
        log_msg(rest->my_node.name, "wanted to cook but Ingredients not enough");
        return 0;
    }

    for(int i=0; i<rest->foods.food[findFoodIdx].ings.numOfIngs;i++){
        for(int j=0;j<rest->ingredients.arrSize;j++){
            if(strcmp(rest->foods.food[findFoodIdx].ings.arr[i].name, rest->ingredients.arr[j].name)==0){
                rest->ingredients.arr[j].amount -= rest->foods.food[findFoodIdx].ings.arr[i].amount;
            }
        }
    }
    return 1;
}

void closeConnection(struct resturant *my_rest, int index, fd_set* master_set){
    close(my_rest->orders[index].fd);
    FD_CLR(my_rest->orders[index].fd, master_set);
    my_rest->orders[index].fd = EMPTY_NUMBER;
}

void answerRequest(struct resturant *my_rest, fd_set* master_set){
    int index = EMPTY_NUMBER;
    for(int i=0;i<my_rest->orders_size;i++){
        if(my_rest->orders[i].status==AWAKE){
            index = i;
            break;
        }
    }
    if(index == EMPTY_NUMBER){
        printf("NO Requests!\n");
        return;
    }
    char buffer[1024] = {0};
        printf("port of request: %d\n", my_rest->orders[index].port);
        printf("your answer(yes/no)\n");
        memset(buffer, 0, 1024);
        read(0, buffer, 1024);
        char message[1024] = "";
        strcat(message, my_rest->my_node.name);
        strcat(message, " Resturant ");
        int canCook = 0;
        if(strcmp(buffer, "yes\n")==0){
            canCook = tryCook(my_rest, index);
            if(canCook){
                strcat(message, "accepted and your food is ready!");
                log_msg(my_rest->my_node.name, "food request accepted");
                send_tcp(my_rest->orders[index].fd, ACCEPT_CODE, message, EMPTY_NUMBER, EMPTY_STRING, EMPTY_NUMBER);
                my_rest->orders[index].status = ACCEPTED;
                closeConnection(my_rest, index, master_set);
            }
        }
        if(strcmp(buffer, "no\n")==0 || !canCook){
            strcat(message, "denied and cry about it!");
            log_msg(my_rest->my_node.name, "food request denied");
            send_tcp(my_rest->orders[index].fd, REJECT_CODE, message, EMPTY_NUMBER, EMPTY_STRING, EMPTY_NUMBER);
            my_rest->orders[index].status = REJECTED;
            closeConnection(my_rest, index, master_set);
        }
        
}



void showSalesHistory(struct resturant *rest){
    for(int i=0; i<rest->orders_size; i++){
        if(rest->orders[i].status != AWAKE){
            char print[1024] = "";
            strcat(print, rest->orders[i].name);
            strcat(print, " ");
            strcat(print, rest->orders[i].food_name);
            strcat(print, " ");
            if(rest->orders[i].status == ACCEPTED){
                strcat(print, "accepted\n");
            }
            else if(rest->orders[i].status == REJECTED){
                strcat(print, "rejected\n");
            }
            else if(rest->orders[i].status == TIMEOUT){
                strcat(print, "timeout\n");
            }
            else{
                strcat(print, "wtf HABIBI");
            }
            write(1, print, strlen(print));
        }
    }
}

void read_command_resturant(struct resturant *my_rest, char* command, fd_set* master_set){
    if(strcmp(command, "start working\n")==0){
        if(my_rest->working == 1)
            return;
        char message[1024] = "";
        strcat(message, my_rest->my_node.name);
        strcat(message, " Resturant opened!\n");
        log_msg(my_rest->my_node.name, "opened resturant");
        send_broadcast(my_rest->my_node, SHOW_MESSAGE, message, EMPTY_NUMBER, EMPTY_STRING, EMPTY_NUMBER);
        my_rest->working=1;
    }
    else if(strcmp(command, "break\n")==0){
        if(my_rest->working == 0)
            return;
        char message[1024] = "";
        strcat(message, my_rest->my_node.name);
        strcat(message, " Resturant closed!\n");
        log_msg(my_rest->my_node.name, "closed Resturant");
        send_broadcast(my_rest->my_node, SHOW_MESSAGE, message, EMPTY_NUMBER, EMPTY_STRING, EMPTY_NUMBER);
        my_rest->working=0;
    }
    else{
        if(my_rest->working == 0){
            printf("You are closed\n");
            return;
        }
    }
    if(strcmp(command, "show ingredients\n")==0){
        printIngredients(*my_rest);
    }
    else if(strcmp(command, "show recipes\n")==0){
        printFoods(*my_rest);
    }
    else if(strcmp(command, "show suppliers\n")==0){
        send_broadcast(my_rest->my_node, SHOW_SUPPLIERS, EMPTY_STRING, my_rest->my_node.tcp_port, EMPTY_STRING, EMPTY_NUMBER);
    }
    else if(strcmp(command, "request ingredient\n")==0){
        requestIngr(my_rest);
    }
    else if(strcmp(command, "show requests list\n")==0){
        showRequests(my_rest);
    }
    else if(strcmp(command, "answer request\n")==0){
        answerRequest(my_rest, master_set);
    }
    else if(strcmp(command, "show sales history\n")==0){
        showSalesHistory(my_rest);
    }
}

void timeoutRequest(struct resturant* rest, int fd){
    for(int i=0;i<rest->orders_size;i++){
        if(rest->orders[i].fd == fd){
            rest->orders[i].status = TIMEOUT;
            rest->orders[i].fd = EMPTY_NUMBER;
        }
    }
}

void process(struct resturant rest){
    int max_sd;
    fd_set master_set, working_set;
    FD_ZERO(&master_set);
    FD_SET(0, &master_set);
    FD_SET(rest.my_node.fd, &master_set);
    FD_SET(rest.my_node.tcp_fd, &master_set);
    if(rest.my_node.tcp_fd>rest.my_node.fd)
        max_sd = rest.my_node.tcp_fd;
    else
        max_sd = rest.my_node.fd;
    char buffer[1024] = {0};

    while (1) {
        working_set = master_set;
        select(max_sd + 1, &working_set, NULL, NULL, NULL);

        for (int i = 0; i <= max_sd; i++) {
            if (FD_ISSET(i, &working_set)) {
                if(i == 0){
                    memset(buffer, 0, 1024);
                    read(0, buffer, 1024);
                    read_command_resturant(&rest, buffer, &master_set);
                }
                else if(i == rest.my_node.fd){
                    memset(buffer, 0, 1024);
                    recv(rest.my_node.fd, buffer, 1024, 0);
                    read_message_resturant(&rest, buffer);
                }
                else if(i == rest.my_node.tcp_fd){
                    memset(buffer, 0, 1024);
                    int fd = accept_tcp(rest.my_node.tcp_fd);
                    recv(fd, buffer, 1024, 0);
                    read_tcp_message_resturant(&rest, buffer, &master_set, fd, &max_sd);
                }
                else{
                    memset(buffer, 0, 1024);
                    int a = recv(i, buffer, 1024, 0);
                    if(a == 0){
                        timeoutRequest(&rest, i);
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
    IngredientsArray ingredients;
    Foods foods;
    getFoods(&foods);
    extractIngredients(&foods, &ingredients);
    struct resturant my_rest;
    my_rest.my_node = setupNode(port, RESTURANT);
    my_rest.working = 1;
    my_rest.ingredients = ingredients;
    my_rest.foods = foods;
    my_rest.orders = (struct order*)malloc(MAX_ORDER_SIZE*sizeof(struct order));
    my_rest.orders_size = 0;
    process(my_rest);
}