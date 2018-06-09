#include <map>
#include "char.h"


static const std::map<std::string, char> charnames(
    {
        {"tab",      '\t'},
        {"return",   '\r'},
        {"linefeed", '\n'},
        {"newline",  '\n'},
        {"page",     '\f'},
        {"nul",      0},
        {"soh",      1},
        {"stx",      2},
        {"etx",      3},
        {"eot",      4},
        {"enq",      5},
        {"ack",      6},
        {"bel",      7},
        {"bs",       8},
        {"ht",       9},
        {"lf",       10},
        {"vt",       11},
        {"ff",       12},
        {"cr",       13},
        {"so",       14},
        {"si",       15},
        {"dle",      16},
        {"dc1",      17},
        {"dc2",      18},
        {"dc3",      19},
        {"dc4",      20},
        {"nak",      21},
        {"syn",      22},
        {"etb",      23},
        {"can",      24},
        {"em",       25},
        {"sub",      26},
        {"esc",      27},
        {"fs",       28},
        {"gs",       29},
        {"rs",       30},
        {"us",       31},
        {"space",    ' '},
        {"del",      127},
    }
);

static const std::string asciinames[128] = {"NUL", "SOH", "STX", "ETX", "EOT", "ENQ", "ACK", "BEL",
                                            "BS", "HT", "LF", "VT", "FF", "CR", "SO", "SI",
                                            "DLE", "DC1", "DC2", "DC3", "DC4", "NAK", "SYN", "ETB",
                                            "CAN", "EM", "SUB", "ESC", "FS", "GS", "RS", "US",
                                            "Space", [127]="DEL"};

std::string normalize_char_name(std::string s)
{
    if(s.length() > 1)
    {
        for(char &c : s)
            c = tolower(c);
        return charnames.count(s) ? s : "";
    }
    return s;
}

unsigned char char_name_to_char(std::string name)
{
    if(name.length() == 1)
        return name[0];
    name = normalize_char_name(name);
    if(charnames.count(name))
        return charnames.at(name);
    return 0;
}

std::string char_to_char_name(unsigned char c)
{
    if(asciinames[c].empty())
        return std::string(1, c);
    return asciinames[c];
}
