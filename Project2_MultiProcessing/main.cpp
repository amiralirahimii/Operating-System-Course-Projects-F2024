#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

//#include <algorithm>
//#include <cstdlib>
#include <iostream>
#include <vector>
#include <filesystem>
#include <string>
#include <map>

#include "logger.hpp"
#include "consts.hpp"
#include "strutils.hpp"

namespace fs = std::filesystem;

Logger lg("main");

struct Shakhes{
    int meanPerMonth;
    int totalPerMonth;
    int busyPerMonth;
    int busyMeanDiffPerMonth;
    int meanLeastDiffPerMonth;
    int price;
};

int createNamedPipesPath(){
    if (fs::exists(consts::PIPES_PATH)) {
        fs::remove_all(consts::PIPES_PATH);
    }
    if (mkdir(consts::PIPES_PATH, 0777) == -1) {
        lg.error("Could not create pipes directory");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int readFolder(std::string folderPath, std::vector<fs::path>& buildingsPath){
    if (fs::exists(folderPath) && fs::is_directory(folderPath)){
        fs::path directoryPath = folderPath;
        for (const auto& entry : fs::directory_iterator(directoryPath)){
            if (entry.is_directory()) {
                buildingsPath.push_back(entry.path());
            }
        }
    }
    else {
        lg.error("Could not open directory: " + folderPath);
        return EXIT_FAILURE;
    }
    if(buildingsPath.size() == 0){
        lg.error("Empty folder: " + folderPath);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

void showBuildings(std::vector<fs::path> buildingsPath){
    lg.info("There are " + std::to_string(buildingsPath.size()) + " buildings:");
    
    for(int i=0; i<buildingsPath.size(); i++){
        lg.info("Building number " + std::to_string(i+1) + "  ->  " + find_name(buildingsPath[i].string()));
    }
}

std::string codeBuildings(std::vector<fs::path> buildingsPath){
    std::string result = "";
    for(int i=0; i<buildingsPath.size(); i++){
        result += find_name(buildingsPath[i].string()) + consts::INNER_PARSER;
    }
    return result;
}

void updateMap(std::map<std::string, std::map<std::string, std::map<int, struct Shakhes>>>& allData, std::string message){
    std::vector<std::string> stage1 = parse_line(message, consts::FINAL_PARSER);

    for(int i=0; i<consts::NUMBER_OF_RESOURSES; i++){
        std::vector<std::string> stage2 = parse_line(stage1[i], consts::NAME_PARSER);
        std::string name = stage2[2];
        std::string resource = find_name(stage2[1]);
        std::vector<std::string> stage3 = parse_line(stage2[0], consts::OUTER_PARSER);
        for(int j=1; j<=consts::NUMBER_OF_MONTH_IN_YEAR; j++){
            std::vector<std::string> stage4 = parse_line(stage3[j-1], consts::INNER_PARSER);
            allData[name][resource][j].meanPerMonth = stoi(stage4[0]);
            allData[name][resource][j].totalPerMonth = stoi(stage4[1]);
            allData[name][resource][j].busyPerMonth = stoi(stage4[2]);
            allData[name][resource][j].busyMeanDiffPerMonth = stoi(stage4[3]);
            allData[name][resource][j].meanLeastDiffPerMonth = stoi(stage4[4]);
            allData[name][resource][j].price = stoi(stage4[5]);
        }
    }
}

void printShakhes(struct Shakhes mySh){
    std::cout << ANSI_CYN << "    Mean Usage: " << ANSI_RST << mySh.meanPerMonth << std::endl;
    std::cout << ANSI_MAG << "    Total Usage: " << ANSI_RST << mySh.totalPerMonth << std::endl;
    std::cout << ANSI_YEL << "    Busy Hour: " << ANSI_RST << mySh.busyPerMonth << std::endl;
    std::cout << ANSI_GRN <<  "    Difference of Busy Hour Usage and Mean: " << ANSI_RST << mySh.busyMeanDiffPerMonth << std::endl;
    std::cout << ANSI_RED << "    Price: " << ANSI_RST << mySh.price << std::endl;
    std::cout << std::endl;
}

void handleUser(std::map<std::string, std::map<std::string, std::map<int, struct Shakhes>>>& allData, std::vector<fs::path> buildingsPath){
    std::cout << ANSI_CYN << "\nThe Buildings Name Are:" << ANSI_RST << "\n";
    for(int i=0; i<buildingsPath.size(); i++){
        std::cout << find_name(buildingsPath[i].string()) << " ";
    }
    std::cout << "\n";
    std::cout << ANSI_CYN << "The Resources Are:" << "\n" << ANSI_RST << consts::WATER_USER_INPUT << " " << consts::GAS_USER_INPUT << " " 
            << consts::ELECTRICTY_USER_INPUT <<"\n";
    std::cout << ANSI_RED << "Choose 1 or more Building:(use space between them)" << ANSI_RST <<"\n";
    std::string line;
    std::getline(std::cin, line);
    std::vector<std::string> buildings = parse_line(line, ' ');
    std::cout << ANSI_BLU <<  "Choose 1 or more Resources:(use space between them)" << ANSI_RST<< "\n";
    std::getline(std::cin, line);
    std::vector<std::string> resources = parse_line(line, ' ');
    std::cout << ANSI_YEL << "Choose 1 or more Months:(use space between them)" << ANSI_RST << "\n";
    std::getline(std::cin, line);
    std::vector<std::string> monthsString = parse_line(line, ' ');
    std::vector<int> months;
    for(int i=0; i<monthsString.size(); i++){
        months.push_back(stoi(monthsString[i]));
    }

    for(int i=0; i<buildings.size(); i++){
        for(int j=0; j<resources.size(); j++){
            for(int k=0; k<months.size(); k++){
                std::cout << ANSI_BLU << buildings[i] << " " << resources[j] << " " << "Month-" <<months[k] << ANSI_RST << "\n";
                printShakhes(allData[buildings[i]][resources[j]][months[k]]);
            }
        }
    }
}

int run(std::vector<fs::path> buildingsPath, char* folderPath){
    showBuildings(buildingsPath);

    int pipe_m_to_b[buildingsPath.size()][2];

    for(int i=0; i<buildingsPath.size(); i++){
        if(pipe(pipe_m_to_b[i]) == -1){
            lg.error("Could not create main to building unnamed pipe");
            return EXIT_FAILURE;
        }
    }

    for(int i=0; i<buildingsPath.size(); i++){
        int pid = fork();
        if(pid < 0){
            lg.error("Could not create process for building: " + buildingsPath[i].string());
        }
        else if(pid == 0){ //child process :: building
            char argv[2][consts::BUFFER_SIZE];
            sprintf(argv[0], "%s", buildingsPath[i].c_str());
            sprintf(argv[1], "%d", pipe_m_to_b[i][1]);
            close(pipe_m_to_b[i][0]);
            if (execl(consts::BUILDING_EXE, consts::BUILDING_EXE, argv[0], argv[1], NULL) == -1) {
                lg.error("Could not execute " + buildingsPath[i].string());
                return EXIT_FAILURE;
            }
        }
        else{
            close(pipe_m_to_b[i][1]);
        }
    }

    int pid = fork();
    if(pid < 0){
        lg.error("Could not create process for bills office");
    }
    else if(pid == 0){ //child process :: billsOffice
        char argv[2][consts::BUFFER_SIZE];
        std::string billsPath = std::string(folderPath) + "/" +std::string(consts::BILLS_FILE_NAME);
        sprintf(argv[0], "%s", billsPath.c_str());
        std::string buildingNames = codeBuildings(buildingsPath);
        sprintf(argv[1], "%s", buildingNames.c_str());
        if (execl(consts::BILLS_EXE, consts::BILLS_EXE, argv[0], argv[1], NULL) == -1) {
            lg.error("Could not execute bills");
            return EXIT_FAILURE;
        }
    }

    for(int i=0; i<buildingsPath.size()+1; i++){
        int status;
        wait(&status);
    }

    lg.info("Finished Waiting for all childs");

    //name -> resource -> month -> shakhes
    std::map<std::string, std::map<std::string, std::map<int, struct Shakhes>>> allData;
    for(int i=0; i<buildingsPath.size(); i++){
        char buffer[consts::BUFFER_SIZE];
        int read_size = read(pipe_m_to_b[i][0], buffer, consts::BUFFER_SIZE);
        lg.warning("Reading from building process pipe... building name: " + find_name(buildingsPath[i].string()));
        buffer[read_size] = '\0';
        updateMap(allData, std::string(buffer));
        close(pipe_m_to_b[i][0]);
    }

    handleUser(allData, buildingsPath);
    return EXIT_SUCCESS;
}

int main(int argc, char* argv[]){
    if (argc != 2) {
        std::cerr << "usage: " << "./main.out" << " <buildings folder>\n";
        return EXIT_FAILURE;
    }
    if(createNamedPipesPath())
        return EXIT_FAILURE;
    
    std::vector<fs::path> buildingsPath;
    if(readFolder(argv[1], buildingsPath))
        return EXIT_FAILURE;

    run(buildingsPath, argv[1]);
    return EXIT_SUCCESS;
}