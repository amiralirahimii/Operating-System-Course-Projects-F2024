#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include <fstream>
#include <iostream>
#include <vector>
#include <filesystem>
#include <string>
#include <map>

#include "logger.hpp"
#include "consts.hpp"
#include "strutils.hpp"

Logger lg("billsOffice");

struct Shakhes{
    int meanPerMonth;
    int totalPerMonth;
    int busyPerMonth;
    int busyMeanDiffPerMonth;
    int meanLeastDiffPerMonth;
    int price;
};

class Manage{
public:
    Manage(std::string message, std::map<int, std::vector<int>> CSVTable);
    virtual std::string codeMessage() = 0;
    virtual void calculatePrice() = 0;
protected:
    std::map<int, struct Shakhes> data;
    std::map<int, std::vector<int>> CSVTable;
    void decodeMessage(std::string message);
    std::string initCodeMessage();
};

Manage::Manage(std::string message, std::map<int, std::vector<int>> CSVTable){
    this->CSVTable = CSVTable;
    decodeMessage(message);
}

void Manage::decodeMessage(std::string message){
    std::vector<std::string> stage1 = parse_line(message, consts::OUTER_PARSER);
    for(int i=1; i<=consts::NUMBER_OF_MONTH_IN_YEAR; i++){
        std::vector<std::string> stage2 = parse_line(stage1[i-1], consts::INNER_PARSER);
        data[i].meanPerMonth = stoi(stage2[0]);
        data[i].totalPerMonth = stoi(stage2[1]);
        data[i].busyPerMonth = stoi(stage2[2]);
        data[i].busyMeanDiffPerMonth = stoi(stage2[3]);
        data[i].meanLeastDiffPerMonth = stoi(stage2[4]);
        data[i].price = 0;
    }
}

std::string Manage::initCodeMessage(){
    std::string message = "";
    for(int i=1; i<=consts::NUMBER_OF_MONTH_IN_YEAR; i++){
        message += std::to_string(data[i].meanPerMonth) + consts::INNER_PARSER + std::to_string(data[i].totalPerMonth)
                + consts::INNER_PARSER + std::to_string(data[i].busyPerMonth) + consts::INNER_PARSER + std::to_string(data[i].busyMeanDiffPerMonth)
                + consts::INNER_PARSER + std::to_string(data[i].meanLeastDiffPerMonth) + consts::INNER_PARSER + std::to_string(data[i].price)
                + consts::OUTER_PARSER;
    }
    return message;
}

class ManageElectricity: public Manage{
public:
    ManageElectricity(std::string message, std::map<int, std::vector<int>> CSVTable);
    void calculatePrice();
    std::string codeMessage();
private:
};

ManageElectricity::ManageElectricity(std::string message, std::map<int, std::vector<int>> CSVTable) : Manage(message, CSVTable){
}

void ManageElectricity::calculatePrice(){
    for(int i=1; i<=consts::NUMBER_OF_MONTH_IN_YEAR; i++){
        data[i].price = data[i].meanPerMonth * consts::NUMBER_OF_DAY_IN_MONTH * CSVTable[i][2]
                        + int(0.25 * data[i].busyMeanDiffPerMonth * consts::NUMBER_OF_DAY_IN_MONTH * CSVTable[i][2])
                        - int(0.25 * data[i].meanLeastDiffPerMonth * consts::NUMBER_OF_DAY_IN_MONTH * CSVTable[i][2]);
    }
}

std::string ManageElectricity::codeMessage(){
    std::string message = initCodeMessage();
    message += consts::NAME_PARSER + std::string(consts::ELECTRICTY_FILE_NAME);
    return message;
}

class ManageGas: public Manage{
public:
    ManageGas(std::string message, std::map<int, std::vector<int>> CSVTable);
    void calculatePrice();
    std::string codeMessage();
private:
};

ManageGas::ManageGas(std::string message, std::map<int, std::vector<int>> CSVTable) : Manage(message, CSVTable){
}

void ManageGas::calculatePrice(){
    for(int i=1; i<=consts::NUMBER_OF_MONTH_IN_YEAR; i++){
        data[i].price = data[i].meanPerMonth * consts::NUMBER_OF_DAY_IN_MONTH * CSVTable[i][1];
    }
}

std::string ManageGas::codeMessage(){
    std::string message = initCodeMessage();
    message += consts::NAME_PARSER + std::string(consts::GAS_FILE_NAME);
    return message;
}

class ManageWater: public Manage{
public:
    ManageWater(std::string message, std::map<int, std::vector<int>> CSVTable);
    void calculatePrice();
    std::string codeMessage();
private:
};

ManageWater::ManageWater(std::string message, std::map<int, std::vector<int>> CSVTable) : Manage(message, CSVTable){
}

void ManageWater::calculatePrice(){
    for(int i=1; i<=consts::NUMBER_OF_MONTH_IN_YEAR; i++){
        data[i].price = data[i].meanPerMonth * consts::NUMBER_OF_DAY_IN_MONTH * CSVTable[i][0]
                        + int(0.25 * data[i].busyMeanDiffPerMonth * consts::NUMBER_OF_DAY_IN_MONTH * CSVTable[i][0]);
    }
}

std::string ManageWater::codeMessage(){
    std::string message = initCodeMessage();
    message += consts::NAME_PARSER + std::string(consts::WATER_FILE_NAME);
    return message;
}

std::vector<std::string> decodeInitMessage(std::string message){
    std::vector<std::string> result = parse_line(message, consts::NAME_PARSER);
    return result;
}

std::string finalCodeMessage(std::string message, std::string name){
    message += consts::NAME_PARSER + name;
    return message;
}

void readCSV(std::map<int, std::vector<int>>& CSVTable, std::string path){
    std::ifstream file(path);
    if (!file.is_open()) {
        lg.error("Open file <" + path + "> failed");
    }
    std::string trash;
    std::getline(file, trash);
    std::string line;
    while (std::getline(file, line)) {
        std::vector<std::string> words = parse_line(line, ',');
        int month = stoi(words[1]);
        int waterPrice = stoi(words[2]);
        int gasPrice = stoi(words[3]);
        int elecPrice = stoi(words[4]);
        CSVTable[month] = {waterPrice, gasPrice, elecPrice};
    }
}

int main(int argc, char* argv[]){
    if (argc != 3) {
        lg.error("Wrong number of arguments");
        return EXIT_FAILURE;
    }
    std::string mePath = std::string(argv[1]);
    std::string buildingNamesString = std::string(argv[2]);
    
    std::map<int, std::vector<int>> CSVTable;
    readCSV(CSVTable, mePath);
    

    std::vector<std::string> buildingNames = parse_line(buildingNamesString, consts::INNER_PARSER);

    for(int i=0; i<buildingNames.size(); i++){
        for(int j=0; j<consts::NUMBER_OF_RESOURSES; j++){
            std::string pipeNameRD = std::string(consts::PIPES_PATH) + "/" + std::string(consts::BUILDING_TO_BILL) + buildingNames[i];

            int fdRD = open(pipeNameRD.c_str(), O_RDONLY);
            
            if (fdRD == -1) {
                lg.error("Can't open pipe: " + pipeNameRD);
                return EXIT_FAILURE;
            }
            char buffer[consts::BUFFER_SIZE];
            int read_size = read(fdRD, buffer, consts::BUFFER_SIZE);
            lg.warning("Reading from building named pipe...");
            buffer[read_size] = '\0';
            close(fdRD);

            std::vector<std::string> initDecode = decodeInitMessage(std::string(buffer));

            Manage* myManager;
            if(initDecode[2] == consts::ELECTRICTY_FILE_NAME)
                myManager = new ManageElectricity(initDecode[0], CSVTable);
            else if(initDecode[2] == consts::GAS_FILE_NAME)
                myManager = new ManageGas(initDecode[0], CSVTable);
            else if(initDecode[2] == consts::WATER_FILE_NAME)
                myManager = new ManageWater(initDecode[0], CSVTable);
            
            myManager->calculatePrice();
            std::string initCode = myManager->codeMessage();

            std::string finalCode = finalCodeMessage(initCode, initDecode[1]);

            std::string pipeNameWR = std::string(consts::PIPES_PATH) + "/" + std::string(consts::BILL_TO_BUILDING) + buildingNames[i];
            int fdWR = open(pipeNameWR.c_str(), O_WRONLY);
            if (fdWR == -1) {
                lg.error("Can't open pipe: " + pipeNameWR);
                return EXIT_FAILURE;
            }
            lg.warning("Writing to building named pipe...");
            write(fdWR, finalCode.c_str(), finalCode.size());
            close(fdWR);
            delete myManager;
        }
    }

    lg.info("Process ran successfully");
    return EXIT_SUCCESS;
}