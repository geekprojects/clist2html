#include <regex>
#include <string>
#include <vector>
#include <map>

using namespace std;

bool shouldTrim(unsigned char ch)
{
    return std::isspace(ch) || std::iscntrl(ch);
}

inline void ltrim(std::string &s)
{
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !shouldTrim(ch);
    }));
}

inline void rtrim(std::string &s)
{
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !shouldTrim(ch);
    }).base(), s.end());
}

inline void trim(std::string &s)
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

struct Line
{
    string line;
    vector<string> tokens;
};

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

string encode(string str)
{
    str = std::regex_replace(str, std::regex("\\&"), "&amp;");
    str = std::regex_replace(str, std::regex("\\<"), "&lt;");
    str = std::regex_replace(str, std::regex("\\>"), "&gt;");

    return str;
}

struct Item
{
    string text;
    string textColour;
    string check;
    string checkColour;
    string comment;

    bool hasText()
    {
        return !text.empty() || !check.empty();
    }
};

struct CheckList
{
    string name;
    vector<Item> items;
};

struct CheckLists
{
    vector<shared_ptr<CheckList>> checkLists;
    map<string, string> colours;
    vector<string> comments;
};

CheckLists readCheckLists(std::string file)
{
     //vector<vector<string>> result = readTextFile("../Q4XP_clist.txt", true);
    vector<Line> result = readTextFile(file, true);

    CheckLists checkLists;
    shared_ptr<CheckList> current = nullptr;

    for (auto line : result)
    {
        if (line.tokens.empty())
        {
            continue;
        }
        const string& cmd = line.tokens.at(0);
        if (cmd == "sw_checklist")
        {
            current = make_shared<CheckList>();
            checkLists.checkLists.push_back(current);

            const string& id = line.tokens.at(1);
            if (line.tokens.size() > 2)
            {
                current->name = line.tokens.at(2);
            }
            else
            {
                current->name = id;
            }
        }
        else if (cmd == "sw_item" ||
            cmd == "sw_item_c" ||
            cmd == "sw_iteminfo" ||
            cmd == "sw_itemvoid" ||
            cmd == "sw_itemvoid_c" ||
            cmd == "sw_remark"
            )
        {
            bool colour = cmd.ends_with("_c");
            if (line.tokens.size() > 1)
            {
                string textline = line.tokens.at(1);
                if (cmd.starts_with("sw_itemvoid"))
                {
                    textline = joinToEnd(line.tokens, 1);
                }
                vector<string> textParts = splitString(textline, '|');
                if (!textParts.empty())
                {
                    Item item;
                    item.text = textParts.at(0);
                    if (textParts.size() > 1)
                    {
                        item.check = textParts.at(1);
                    }
                    if (colour && item.text.starts_with("\\"))
                    {
                        item.text = item.text.substr(1);
                        auto idx = item.text.find('\\');
                        if (idx != string::npos)
                        {
                            string colourStr = item.text.substr(0, idx);
                            item.text = item.text.substr(idx + 1);
                            item.textColour = colourStr;
                        }

                        // Remove any colour formatting in the middle of the text
                        item.text = std::regex_replace(item.text, std::regex(R"(\\[a-z]*\\)"), "");
                    }
                    if (colour && item.check.starts_with("\\"))
                    {
                        item.check = item.check.substr(1);
                        auto idx = item.check.find('\\');
                        if (idx != string::npos)
                        {
                            string colourStr = item.check.substr(0, idx);
                            item.check = item.check.substr(idx + 1);
                            item.checkColour = colourStr;
                        }

                        // Remove any colour formatting in the middle of the text
                        item.check = std::regex_replace(item.check, std::regex(R"(\\[a-z]*\\)"), "");
                    }

                    current->items.push_back(item);
                }
            }
        }
        else if (cmd == "sw_define_colour")
        {
            if (line.tokens.size() == 3)
            {
                const string& name = line.tokens.at(1);
                const string& rgbColor = line.tokens.at(2);
                auto rgbParts = splitString(rgbColor, ',');

                int r = (int)(atof(rgbParts.at(0).c_str()) * 255.0);
                int g = (int)(atof(rgbParts.at(1).c_str()) * 255.0);
                int b = (int)(atof(rgbParts.at(2).c_str()) * 255.0);

                char colour[32];
                snprintf(colour, sizeof(colour), "#%02x%02x%02x", r, g, b);

                checkLists.colours.try_emplace(name, colour);
            }
        }
        else if (cmd.starts_with("sw_continue") || cmd.starts_with("sw_rcolsize") || cmd == "sw_show")
        {
            // Ignored
        }
        else if (cmd.starts_with("#"))
        {
            // Comment
            string comment = line.line.substr(1);
            if (!comment.starts_with("sw_"))
            {
                checkLists.comments.push_back(comment);
            }
        }
        else
        {
            fprintf(stderr, "Unknown command: %s\n", cmd.c_str());
            exit(1);
        }
    }

    return checkLists;
}

string cell(
    string text,
    const string& colourName,
    const string& className,
    int colspan,
    const map<string, string>& colours)
{
    string colour;
    if (!colourName.empty())
    {
        auto it = colours.find(colourName);
        if (it != colours.end())
        {
            colour = it->second;
        }
    }
    string td = "<td ";
    if (colspan > 1)
    {
        td += "colspan=" + to_string(colspan) + " ";
    }
    if (!colour.empty())
    {
        td += "bgcolor=\"" + colour + "\" ";
    }
    if (!className.empty())
    {
        td += "class=\"" + className + "\" ";
    }

    if (text.empty())
    {
        text = "&nbsp;";
    }
    else
    {
        text = encode(text);
    }

    td += ">" + text + "</td>";
    return td;
}

int main(int argc, char** argv)
{
    string clistFile;
    if (argc != 2)
    {
        //return 0;
        clistFile = "../clist.txt";
    }
    else
    {
        clistFile = argv[1];
    }


    auto idx = clistFile.find_last_of(".");
    string clistHtml = clistFile.substr(0, idx + 1) + "html";

    auto checkLists = readCheckLists(clistFile);

    FILE* fd = fopen(clistHtml.c_str(), "w");
    if (fd == nullptr)
    {
        fprintf(stderr, "Failed to create output file\n");
        exit(1);
    }
    fprintf(fd, "<!DOCTYPE html>\n");
    fprintf(fd, "<html>\n");
    fprintf(fd, "<head>\n");
    fprintf(fd, "<style>\n");
    fprintf(fd, "html * {font-family: monospace; }\n");
    fprintf(fd, "table, th, td { border: 1px solid black; border-collapse: collapse; }\n");
    fprintf(fd, "tr:nth-child(odd) { background-color: #eeeeee; }\n");
    fprintf(fd, ".checkList { width: 90%%; page-break-inside: avoid; }\n");
    fprintf(fd, ".checkListTitle { background-color: #cccccc; font-size: 1.5em; }\n");
    fprintf(fd, ".itemInfo { text-align: center; }\n");
    fprintf(fd, ".itemTest { text-align: left; }\n");
    fprintf(fd, ".itemCheck { text-align: right ; width: 1%%; white-space: nowrap; }\n");
    fprintf(fd, "</style>\n");
    fprintf(fd, "</head>\n");
    fprintf(fd, "<body>\n");

    for (const auto& comment : checkLists.comments)
    {
        fprintf(fd, "<!-- %s -->\n", encode(comment).c_str());
    }

    for (const auto& checkList : checkLists.checkLists)
    {
        fprintf(fd, "<table class=\"checkList\">\n");
        fprintf(fd, "<tr><th colspan=2 class=\"checkListTitle\">%s</th></tr>\n", encode(checkList->name).c_str());
        for (auto it = checkList->items.begin(); it != checkList->items.end(); it++)
        {
            fprintf(fd, "<tr>");

            auto item = *it;
            string text = item.text;
            if (std::regex_match(text, std::regex("^-*$")))
            {
                text = "";
            }

            if (!item.check.empty())
            {
                fprintf(
                    fd,
                    "%s%s\n",
                    cell(text, item.textColour, "itemText", 1, checkLists.colours).c_str(),
                    cell(item.check, item.checkColour, "itemCheck", 1, checkLists.colours).c_str());
            }
            else
            {
                bool skip = false;
                if (text.empty())
                {
                    if (it != checkList->items.begin())
                    {
                        auto prev = *(it - 1);
                        skip = prev.check.empty();
                    }
                    if ((it + 1) != checkList->items.end())
                    {
                        auto next = *(it + 1);
                        skip |= next.check.empty();
                    }
                }

                if (!skip)
                {
                    fprintf(
                        fd,
                        "%s\n",
                        cell(text, item.textColour, "itemInfo", 2, checkLists.colours).c_str());
                }
            }
            fprintf(fd, "</tr>");
        }
        fprintf(fd, "</table>\n");
        fprintf(fd, "<br>\n");
    }

    fprintf(fd, "</body>");
    fprintf(fd, "</html>");
    fclose(fd);

    return 0;
}
