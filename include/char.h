#ifndef SCHEME_CHAR_H
#define SCHEME_CHAR_H

#include <string>

std::string normalize_char_name(std::string s);
unsigned char char_name_to_char(std::string name);
std::string char_to_char_name(unsigned char c);

#endif //SCHEME_CHAR_H
