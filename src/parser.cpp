#include <vector>
#include "parser.h"

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
    if(isdigit(s[i]))
    {
        o.type = ast_type_t::INT;
        bool dot = false;
        while(i < s.length() && (isdigit(s[i]) || s[i] == '.'))
        {
            if(s[i] == '.')
            {
                if(dot)
                    throw parse_error("Invalid numeric literal");
                else
                    dot = true;
            }
            o.value.push_back(s[i++]);
        }
        if(dot)
            o.type = ast_type_t::FLOAT;
        return i;
    }
    o.type = ast_type_t::NAME;
    while(i < s.length() && s[i] != '(' && s[i] != ')' && !isspace(s[i]))
    {
        o.value.push_back(s[i++]);
    }
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
