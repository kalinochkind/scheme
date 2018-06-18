#ifndef SCHEME_PARSER_H
#define SCHEME_PARSER_H

#include "schemeobject.h"
#include <iostream>
#include <memory>


enum class parse_result_t
{
    OK, ERROR, END
};

struct ParseResult
{
    parse_result_t result;
    std::shared_ptr<ASTNode> node;
    std::string error;

    ParseResult(std::shared_ptr<ASTNode> p): result(parse_result_t::OK), node(std::move(p)), error{} {}
    explicit ParseResult(const std::string &s): result(parse_result_t::ERROR), node(nullptr), error(s) {}
    ParseResult(): result(parse_result_t::END), node(nullptr), error{} {}
};

ParseResult readObject(std::istream &is);
ast_type_t identifier_type(const std::string &s);

#endif //SCHEME_PARSER_H
