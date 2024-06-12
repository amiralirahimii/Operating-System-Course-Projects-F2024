#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <string>
#include <iostream>
#include "colorPrint.hpp"

class Logger {
public:
    Logger(std::string fileName);
    void error(std::string msg);
    void warning(std::string msg);
    void info(std::string msg);

private:
    std::string fileName;
};

#endif