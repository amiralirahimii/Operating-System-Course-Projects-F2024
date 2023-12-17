#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include <iostream>
#include <vector>
#include <filesystem>
#include <string>
#include <map>
#include <fstream>

#include "logger.hpp"
#include "consts.hpp"
#include "strutils.hpp"

Logger lg("resource");

class ResourceHandler{
public:
    ResourceHandler(std::string path);
    std::vector<int> calculateMeanPerMonths();
    std::vector<int> calculateTotalPerMonths();
    std::vector<int> calculateBusyPerMonths();
    std::vector<int> calculateBusyMeanDiffPerMonths();
    std::vector<int> calculateMeanLeastDiffPerMonths();
private:
    std::string path;
    std::map<std::pair<int, int>, std::vector<int>> CSVTable;
    void initTable(const std::vector<std::string>& words);
    void readCSV();
    int calculateMeanThisMonth(int monthNumber);
    int calculateTotalThisMonth(int monthNumber);
    int calculateBusyThisMonth(int monthNumber);
    int findMaxIndex(const std::vector<int>& numbers);
    int calculateBusyMeanDiffThisMonth(int monthNumber);
    int calculateLeastThisMonth(int monthNumber);
    int findMinIndex(const std::vector<int>& numbers);
    int calculateMeanLeastDiffThisMonth(int monthNumber);
};

ResourceHandler::ResourceHandler(std::string path){
    this->path = path;
    readCSV();

}

void ResourceHandler::initTable(const std::vector<std::string>& words){
    int month = stoi(words[1]);
    int day = stoi(words[2]);
    std::vector<int> hours = {stoi(words[3]), stoi(words[4]), stoi(words[5]), stoi(words[6]), stoi(words[7]), stoi(words[8])};
    this->CSVTable[std::make_pair(month, day)] = hours;
}

void ResourceHandler::readCSV(){
    std::ifstream file(this->path);
    if (!file.is_open()) {
        lg.error("Open file <" + this->path + "> failed");
    }

    std::string trash;
    std::getline(file, trash);
    std::string line;
    while (std::getline(file, line)) {
        std::vector<std::string> words = parse_line(line, ',');
        initTable(words);
    }
}

int ResourceHandler::calculateMeanThisMonth(int monthNumber){
    int result = 0;
    for(int i=1; i<=consts::NUMBER_OF_DAY_IN_MONTH; i++){
        for(int j=0; j<consts::NUMBER_OF_HOURS_IN_DAY; j++){
            result += CSVTable[std::make_pair(monthNumber, i)][j];
        }
    }
    result /= consts::NUMBER_OF_DAY_IN_MONTH;
    return result;
}

std::vector<int> ResourceHandler::calculateMeanPerMonths(){
    std::vector<int> result;
    for(int i=1; i<=consts::NUMBER_OF_MONTH_IN_YEAR; i++){
        result.push_back(calculateMeanThisMonth(i));
    }
    return result;
}

int ResourceHandler::calculateTotalThisMonth(int monthNumber){
    int result = 0;
    for(int i=1; i<=consts::NUMBER_OF_DAY_IN_MONTH; i++){
        for(int j=0; j<consts::NUMBER_OF_HOURS_IN_DAY; j++){
            result += CSVTable[std::make_pair(monthNumber, i)][j];
        }
    }
    return result;
}

std::vector<int> ResourceHandler::calculateTotalPerMonths(){
    std::vector<int> result;
    for(int i=1; i<=consts::NUMBER_OF_MONTH_IN_YEAR; i++){
        result.push_back(calculateTotalThisMonth(i));
    }
    return result;
}

int ResourceHandler::findMaxIndex(const std::vector<int>& numbers){
    int maxIndex = 0;
    for(int i=0; i<numbers.size(); i++){
        maxIndex = (numbers[i] > numbers[maxIndex]) ? i : maxIndex;
    }
    return maxIndex;
}

int ResourceHandler::calculateBusyThisMonth(int monthNumber){
    std::vector<int> totalUseInHour(consts::NUMBER_OF_HOURS_IN_DAY, 0);
    for(int i=1; i<=consts::NUMBER_OF_DAY_IN_MONTH; i++){
        for(int j=0; j<consts::NUMBER_OF_HOURS_IN_DAY; j++){
            totalUseInHour[j] += CSVTable[std::make_pair(monthNumber, i)][j];
        }
    }
    return findMaxIndex(totalUseInHour);
}

std::vector<int> ResourceHandler::calculateBusyPerMonths(){
    std::vector<int> result;
    for(int i=1; i<=consts::NUMBER_OF_MONTH_IN_YEAR; i++){
        result.push_back(calculateBusyThisMonth(i));
    }
    return result;
}

int ResourceHandler::calculateBusyMeanDiffThisMonth(int monthNumber){
    int busyHour = calculateBusyThisMonth(monthNumber);
    int busyMeanHourConsumption = 0;
    for(int i=1; i<=consts::NUMBER_OF_DAY_IN_MONTH; i++){
        busyMeanHourConsumption += CSVTable[std::make_pair(monthNumber, i)][busyHour];
    }
    busyMeanHourConsumption /= consts::NUMBER_OF_DAY_IN_MONTH;
    busyMeanHourConsumption *= consts::NUMBER_OF_HOURS_IN_DAY;
    return busyMeanHourConsumption - calculateMeanThisMonth(monthNumber);
}

std::vector<int> ResourceHandler::calculateBusyMeanDiffPerMonths(){
    std::vector<int> result;
    for(int i=1; i<=consts::NUMBER_OF_MONTH_IN_YEAR; i++){
        result.push_back(calculateBusyMeanDiffThisMonth(i));
    }
    return result;
}

int ResourceHandler::findMinIndex(const std::vector<int>& numbers){
    int minIndex = 0;
    for(int i=0; i<numbers.size(); i++){
        minIndex = (numbers[i] < numbers[minIndex]) ? i : minIndex;
    }
    return minIndex;
}

int ResourceHandler::calculateLeastThisMonth(int monthNumber){
    std::vector<int> totalUseInHour(consts::NUMBER_OF_HOURS_IN_DAY, 0);
    for(int i=1; i<=consts::NUMBER_OF_DAY_IN_MONTH; i++){
        for(int j=0; j<consts::NUMBER_OF_HOURS_IN_DAY; j++){
            totalUseInHour[j] += CSVTable[std::make_pair(monthNumber, i)][j];
        }
    }
    return findMinIndex(totalUseInHour);
}

int ResourceHandler::calculateMeanLeastDiffThisMonth(int monthNumber){
    int leastHour = calculateLeastThisMonth(monthNumber);
    int meanLeastHourConsumption = 0;
    for(int i=1; i<=consts::NUMBER_OF_DAY_IN_MONTH; i++){
        meanLeastHourConsumption += CSVTable[std::make_pair(monthNumber, i)][leastHour];
    }
    meanLeastHourConsumption /= consts::NUMBER_OF_DAY_IN_MONTH;
    meanLeastHourConsumption *= consts::NUMBER_OF_HOURS_IN_DAY;

    return calculateMeanThisMonth(monthNumber) - meanLeastHourConsumption;
}

std::vector<int> ResourceHandler::calculateMeanLeastDiffPerMonths(){
    std::vector<int> result;
    for(int i=1; i<=consts::NUMBER_OF_MONTH_IN_YEAR; i++){
        result.push_back(calculateMeanLeastDiffThisMonth(i));
    }
    return result;
}


std::string runAndCodeMsg(ResourceHandler& resourceHandler){
    std::vector<int> CMPM = resourceHandler.calculateMeanPerMonths();
    std::vector<int> CTPM = resourceHandler.calculateTotalPerMonths();
    std::vector<int> CBPM = resourceHandler.calculateBusyPerMonths();
    std::vector<int> CBMDPM = resourceHandler.calculateBusyMeanDiffPerMonths();
    std::vector<int> CMLDPM = resourceHandler.calculateMeanLeastDiffPerMonths();

    std::string message = "";
    for(int i=0; i<consts::NUMBER_OF_MONTH_IN_YEAR; i++){
        message += std::to_string(CMPM[i]) + consts::INNER_PARSER + std::to_string(CTPM[i]) + consts::INNER_PARSER + std::to_string(CBPM[i])
                + consts::INNER_PARSER + std::to_string(CBMDPM[i]) + consts::INNER_PARSER + std::to_string(CMLDPM[i]) + consts::OUTER_PARSER;
    }
    return message;
}


int main(int argc, char* argv[]){
    if (argc != 3) {
        lg.error("Wrong number of arguments");
        return EXIT_FAILURE;
    }
    std::string mePath = std::string(argv[1]);
    int write_fd = atoi(argv[2]);

    ResourceHandler resourceHandler(mePath);
    
    std::string message = runAndCodeMsg(resourceHandler);
    
    lg.warning("Writing to building process pipe...");
    write(write_fd, message.c_str(), message.size());
    close(write_fd);
    lg.info("Process ran successfully");
    return EXIT_SUCCESS;
}