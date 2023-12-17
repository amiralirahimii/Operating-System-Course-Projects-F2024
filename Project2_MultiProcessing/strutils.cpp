#include "strutils.hpp"

std::vector<std::string> parse_line(std::string line, char parser) {
    std::vector<std::string> result;
    std::string seprating_word;
    for (int i = 0; i < line.size(); i++) {
        if (line[i] == parser && seprating_word.size()!=0) {
            result.push_back(seprating_word);
            seprating_word = "";
        }
        else {
            seprating_word += line[i];
        }
    }
    if (seprating_word.size())
        result.push_back(seprating_word);
    return result;
}

std::string find_name(std::string path){
    std::vector<std::string> parsed = parse_line(path, '/');
    std::string lastPart = parsed[parsed.size()-1];
    std::vector<std::string> parsed2 = parse_line(lastPart, '.');
    // can change this.
    return parsed2[0];
}