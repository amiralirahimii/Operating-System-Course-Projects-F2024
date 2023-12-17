#include "logger.hpp"

Logger::Logger(std::string fileName){
    this->fileName = fileName;
}

void Logger::error(std::string msg) {
    std::cerr << Color::RED << "ERROR from " << fileName << ": " <<  Color::RST << msg << std::endl;
}

void Logger::warning(std::string msg) {
    std::cout << Color::YEL << "WARNING from " << fileName << ": " << Color::RST << msg << std::endl;
}

void Logger::info(std::string msg) {
    std::cout << Color::GRN << "INFO from " << fileName << ": " << Color::RST << msg << std::endl;
}
