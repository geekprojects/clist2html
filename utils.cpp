//
// Created by Ian Parker on 24/06/2025.
//

#include "utils.h"

using namespace std;

bool shouldTrim(unsigned char ch)
{
    return std::isspace(ch) || std::iscntrl(ch);
}

void ltrim(std::string &s)
{
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !shouldTrim(ch);
    }));
}

void rtrim(std::string &s)
{
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !shouldTrim(ch);
    }).base(), s.end());
}

void trim(std::string &s)
{
    rtrim(s);
    ltrim(s);
}

vector<string> splitString(string line, char splitChar)
{
    vector<string> parts;

    while (!line.empty())
    {
        size_t pos = line.find(splitChar);
        if (pos == string::npos)
        {
            pos = line.length();
            if (pos == 0)
            {
                break;
            }
        }
        if (pos >= 1)
        {
            string part = line.substr(0, pos);
            trim(part);
            parts.push_back(part);
        }
        if (pos == line.length())
        {
            break;
        }
        line = line.substr(pos + 1);
    }

    return parts;
}

std::string joinToEnd(vector<string> parts, int startPos)
{
    std::string result;
    for (unsigned int i = startPos; i < parts.size(); i++)
    {
        if (!result.empty())
        {
            result += " ";
        }
        result += parts.at(i);
    }
    return result;
}


vector<Line> readTextFile(const string& filename, bool split)
{
    vector<Line> result;
    FILE* fd = fopen(filename.c_str(), "r");
    if (fd == nullptr)
    {
        fprintf(stderr, "readTextFile: Failed to open file %s\n", filename.c_str());
        return result;
    }

    char lineBuffer[2048];
    while (fgets(lineBuffer, 2048, fd) != nullptr)
    {
        Line line;
        line.line = lineBuffer;

        // Skip UTF-8 BOM
        if (line.line.length() >= 3 && (uint8_t)line.line.at(0) == 0xef && (uint8_t)line.line.at(1) == 0xbb && (uint8_t)line.line.at(2) == 0xbf)
        {
            line.line = line.line.substr(3);
        }

        trim(line.line);
        if (split)
        {
            line.tokens = splitString(line.line, ':');
        }
        result.push_back(line);
    }
    fclose(fd);
    return result;
}
