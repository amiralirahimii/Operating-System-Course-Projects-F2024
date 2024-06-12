#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h>
#include <arpa/inet.h> 
#include <netinet/in.h> 
#include <netdb.h>

#include "logger.h"
#include "define.h"

void creating_log_file(char* name)
{
    char path[BUF_SIZE];
    sprintf(path, "%s.log", name);

    int fd = creat( path, RD_WR );
    if(fd == EMPTY_NUMBER) return;

    close(fd);
}

void log_msg(char* name, char* msg)
{
    char path[BUF_SIZE];
    sprintf(path, "%s.log", name);

    int fd = open(path, O_WRONLY | O_APPEND);

    char buf[BUF_SIZE];
    sprintf(buf, "%s\n", msg);
    write(fd, buf, strlen(buf));

    close(fd);
}