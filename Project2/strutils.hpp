#ifndef STRUTILS_HPP
#define STRUTILS_HPP

#include <vector>
#include <string>

std::vector<std::string> parse_line(std::string line, char parser);
std::string find_name(std::string path);
#endif