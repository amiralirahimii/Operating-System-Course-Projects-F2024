#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include <iostream>
#include <vector>
#include <filesystem>
#include <string>
#include <map>

#include "logger.hpp"
#include "consts.hpp"
#include "strutils.hpp"

Logger lg("building");

void initMap(std::map<int, std::string>& resourceMap){
    resourceMap[0] = consts::ELECTRICTY_FILE_NAME;
    resourceMap[1] = consts::GAS_FILE_NAME;
    resourceMap[2] = consts::WATER_FILE_NAME;
}

int makeNamedPipes(std::string mePath){
    std::string pipeNameRD = std::string(consts::PIPES_PATH) + "/" + std::string(consts::BILL_TO_BUILDING) + find_name(mePath);
    if (mkfifo(pipeNameRD.c_str(), 0666) == -1) {
        lg.error("Can't create named pipe for " + find_name(mePath));
        return EXIT_FAILURE;
    }

    std::string pipeNameWR = std::string(consts::PIPES_PATH) + "/" + std::string(consts::BUILDING_TO_BILL) + find_name(mePath);
    if (mkfifo(pipeNameWR.c_str(), 0666) == -1) {
        lg.error("Can't create named pipe for " + find_name(mePath));
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

std::string addMyNameAndResource(char buffer[], std::string mePath, std::string resource){
    std::string result;
    result = std::string(buffer);
    result += consts::NAME_PARSER + find_name(mePath) + consts::NAME_PARSER + resource;
    return result;
}

int run(std::string mePath, int write_fd){
    std::map<int, std::string> resourceMap;
    initMap(resourceMap);

    int pipe_b_to_r[consts::NUMBER_OF_RESOURSES][2];
    for(int i=0; i<consts::NUMBER_OF_RESOURSES; i++){
        if(pipe(pipe_b_to_r[i]) == -1){
            lg.error("Could not create building to resourse unnamed pipe " + resourceMap[i]);
            return EXIT_FAILURE;
        }
    }

    makeNamedPipes(mePath);

    for(int i=0; i<consts::NUMBER_OF_RESOURSES; i++){
        int pid = fork();
        if(pid < 0){
            lg.error("Could not create process for resource: " + std::to_string(consts::NUMBER_OF_RESOURSES));
            //need to return????
        }
        else if(pid == 0){ //child process :: building
            char argv[2][consts::BUFFER_SIZE];
            std::string resourcePath = mePath + '/' + resourceMap[i];
            sprintf(argv[0], "%s", resourcePath.c_str());
            sprintf(argv[1], "%d", pipe_b_to_r[i][1]);
            close(pipe_b_to_r[i][0]);
            if (execl(consts::RESOURCE_EXE, consts::RESOURCE_EXE, argv[0], argv[1], NULL) == -1) {
                lg.error("Could not execute " + resourceMap[i]);
                return EXIT_FAILURE;
            }
        }
        else{
            close(pipe_b_to_r[i][1]);
        }
    }

    for(int i=0; i<consts::NUMBER_OF_RESOURSES; i++){
        int status;
        wait(&status);
        lg.info("wait for resource processes finished in building " + find_name(mePath) + find_name(resourceMap[i]));
    }

    std::string pipeNameWR = std::string(consts::PIPES_PATH) + "/" + std::string(consts::BUILDING_TO_BILL) + find_name(mePath);
    std::string pipeNameRD = std::string(consts::PIPES_PATH) + "/" + std::string(consts::BILL_TO_BUILDING) + find_name(mePath);

    std::string totalMessage = "";
    for(int i=0; i<consts::NUMBER_OF_RESOURSES; i++){
        char buffer[consts::BUFFER_SIZE];
        int read_size1 = read(pipe_b_to_r[i][0], buffer, consts::BUFFER_SIZE);
        lg.warning("Reading from resource process pipe... resource name: " + find_name(resourceMap[i]));
        buffer[read_size1] = '\0';
        
        int fdWR = open(pipeNameWR.c_str(), O_WRONLY);
        if (fdWR == -1) {
            lg.error("Can't open pipe: " + pipeNameWR);
            return EXIT_FAILURE;
        }
        
        std::string message = addMyNameAndResource(buffer, mePath, resourceMap[i]);
        write(fdWR, message.c_str(), message.size());
        lg.warning("Writing to billsOffice named pipe...");
        
        close(fdWR);
        

        int fdRD = open(pipeNameRD.c_str(), O_RDONLY);
        if (fdRD == -1) {
            lg.error("Can't open pipe: " + pipeNameRD);
            return EXIT_FAILURE;
        }
        char buffer2[consts::BUFFER_SIZE];
        int read_size2 = read(fdRD, buffer2, consts::BUFFER_SIZE);
        lg.warning("Reading from billsOffice named pipe...");
        buffer2[read_size2] = '\0';
        close(fdRD);
        totalMessage += std::string(buffer2) + consts::FINAL_PARSER;
    }

    lg.warning("Writing to main process pipe...");
    write(write_fd, totalMessage.c_str(), totalMessage.size());
    close(write_fd);
    return EXIT_SUCCESS;
}

int main(int argc, char* argv[]){
    if (argc != 3) {
        lg.error("Wrong number of arguments");
        return EXIT_FAILURE;
    }
    std::string mePath = std::string(argv[1]);
    int write_fd = atoi(argv[2]);

    if(run(mePath, write_fd))
        return EXIT_FAILURE;

    lg.info("Process ran successfully");
    return EXIT_SUCCESS;
}