#include <regex>
#include <string>
#include <vector>
#include <map>

#include <getopt.h>

#include "utils.h"

using namespace std;

string encode(string str)
{
    str = std::regex_replace(str, std::regex("\\&"), "&amp;");
    str = std::regex_replace(str, std::regex("\\<"), "&lt;");
    str = std::regex_replace(str, std::regex("\\>"), "&gt;");

    // Truncate long strings used for horizontal rules, titles etc
    str = std::regex_replace(str, std::regex("([-_=]){3,}"), "$1$1$1");

    return str;
}

struct Item
{
    string text;
    string textColour;
    string check;
    string checkColour;
    string comment;

    [[nodiscard]] bool hasText() const
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
    CheckLists checkLists;
    vector<Line> result = readTextFile(file, true);
    if (result.empty())
    {
        return checkLists;
    }

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

[[noreturn]] void usage(string argv0, int e)
{
    fprintf(stderr, "Usage: %s [options] <filename>\n", argv0.c_str());
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  --columns <n>       Number of columns to format lists in to (Optional)\n");
    fprintf(stderr, "  --title <title>     Title to display at the top (Optional)\n");
    fprintf(stderr, "  --output <filename> Filename of the output file (Optional)\n");

    exit(e);
}

int main(int argc, char** argv)
{
    int columns = 2;
    string title;
    string clistHtml;
    string argv0 = argv[0];

    string optstr = "c:t:o:h";
    vector<option> options;
    options.push_back({"columns", required_argument, 0,  'c'});
    options.push_back({"title", required_argument, 0,  't'});
    options.push_back({"output", required_argument, 0,  'o'});
    options.push_back({"help", no_argument, 0,  'h'});

    optind = 0;
    int ch;
    while ((ch = getopt_long(argc, argv, optstr.c_str(), options.data(), nullptr)) != -1)
    {
        switch (ch)
        {
            case 'c':
                columns = atoi(optarg);
                break;
            case 't':
                title = optarg;
                break;
            case 'o':
                clistHtml = optarg;
                break;
            case 'h':
                usage(argv0, 0);
            default:
                usage(argv0, 1);
        }
    }

    argc -= optind;
    argv += optind;

    if (argc < 1)
    {
        usage(argv0, 1);
    }

    string clistFile = argv[0];
    if (clistHtml.empty())
    {
        auto idx = clistFile.find_last_of('.');
        if (idx != string::npos)
        {
            clistHtml = clistFile.substr(0, idx + 1) + "html";
        }
        else
        {
            clistHtml = clistFile + ".html";
        }
    }

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
    fprintf(fd, "table, th, td { border: 1px solid black; border-collapse: collapse; margin-bottom: 20px; }\n");
    fprintf(fd, "tr:nth-child(odd) { background-color: #eeeeee; }\n");
    fprintf(fd, ".title { font-size: 2em; }\n");
    fprintf(fd, ".checkListContainer { column-count: %d; }\n", columns);
    fprintf(fd, ".checkList { width: 90%%; page-break-inside: avoid; }\n");
    fprintf(fd, ".checkListTitle { background-color: #000; color: #ffffff; font-size: 1.3em; }\n");
    fprintf(fd, ".itemInfo { text-align: center; }\n");
    fprintf(fd, ".itemTest { text-align: left; }\n");
    fprintf(fd, ".itemCheck { text-align: right ; width: 1%%; white-space: nowrap; }\n");
    fprintf(fd, "</style>\n");
    fprintf(fd, "</head>\n");
    fprintf(fd, "<body>\n");

    if (!title.empty())
    {
        fprintf(fd, "<h1 class=\"title\">%s</h1>\n", title.c_str());
    }

    fprintf(fd, "<div class=\"checkListContainer\">\n");
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
    }

    fprintf(fd, "</div>");
    fprintf(fd, "</body>");
    fprintf(fd, "</html>");
    fclose(fd);

    return 0;
}
