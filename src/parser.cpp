#include <vector>
#include <regex>
#include "parser.h"

static ast_type_t identifier_type(const std::string &s)
{
    static const std::regex float_re("^-?(\\d+\\.\\d*|\\d*\\.\\d+)$");
    static const std::regex int_re("^-?\\d+$");
    if(std::regex_match(s, int_re))
        return ast_type_t::INT;
    if(std::regex_match(s, float_re))
        return ast_type_t::FLOAT;
    return ast_type_t::NAME;
}

static size_t _parse(const std::string &s, ASTNode &o, size_t i)
{
    while(i < s.length() && isspace(s[i]))
        ++i;
    if(i == s.length())
        return 0;
    if(s[i] == ')')
        throw parse_error("Invalid syntax");
    if(s[i] == '"')
    {
        o.type = ast_type_t::STRING;
        ++i;
        while(i < s.length() && s[i] != '"')
        {
            o.value.push_back(s[i]);
            ++i;
        }
        if(i >= s.length())
            throw parse_error("Unclosed string literal");
        return ++i;
    }
    if(s[i] == '(')
    {
        o.type = ast_type_t::LIST;
        ++i;
        while(i < s.length() && s[i] != ')')
        {
            auto p = std::make_unique<ASTNode>();
            i = _parse(s, *p, i);
            o.list.push_back(std::move(p));
            while(i < s.length() && isspace(s[i]))
                ++i;
        }
        if(i == s.length())
            throw unexpected_eol_error("Unclosed list literal");
        return ++i;
    }
    while(i < s.length() && s[i] != '(' && s[i] != ')' && !isspace(s[i]))
    {
        o.value.push_back(s[i++]);
    }
    o.type = identifier_type(o.value);
    return i;
}


std::vector<ASTNode> parseString(const std::string &s)
{
    size_t i = 0;
    std::vector<ASTNode> res;
    do
    {
        ASTNode o;
        i = _parse(s, o, i);
        if(i)
            res.push_back(std::move(o));
    } while(i);
    return res;
}
