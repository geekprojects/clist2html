//
// Created by Ian Parker on 24/06/2025.
//

#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>

struct Line
{
    std::string line;
    std::vector<std::string> tokens;
};

bool shouldTrim(unsigned char ch);
void ltrim(std::string &s);
void rtrim(std::string &s);
void trim(std::string &s);
std::vector<std::string> splitString(std::string line, char splitChar);
std::string joinToEnd(std::vector<std::string> parts, int startPos);
std::vector<Line> readTextFile(const std::string& filename, bool split);



#endif //UTILS_H
