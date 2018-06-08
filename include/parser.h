#ifndef SCHEME_PARSER_H
#define SCHEME_PARSER_H

#include "schemeobject.h"
#include <iostream>
#include <memory>

class parse_error: public std::runtime_error
{
    using std::runtime_error::runtime_error;
};

class end_of_input: public parse_error
{
    using parse_error::parse_error;
};

std::shared_ptr<ASTNode> readObject(std::istream &is);
ast_type_t identifier_type(const std::string &s);

#endif //SCHEME_PARSER_H
