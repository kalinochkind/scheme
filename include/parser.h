#ifndef SCHEME_PARSER_H
#define SCHEME_PARSER_H

#include "schemeobject.h"
#include <vector>
#include <string>

class parse_error: public std::runtime_error
{
    using std::runtime_error::runtime_error;
};

class unexpected_eol_error: public parse_error
{
    using parse_error::parse_error;
};

std::vector<ASTNode> parseString(const std::string &s);

#endif //SCHEME_PARSER_H
