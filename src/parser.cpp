#include <regex>
#include <string>
#include "parser.h"
#include "char.h"

ast_type_t identifier_type(const std::string &s)
{
    static const std::regex float_re("^-?(\\d+\\.\\d*|\\d*\\.\\d+)$");
    static const std::regex rational_re("^-?\\d+\\/-?\\d+$");
    static const std::regex int_re("^-?\\d+$");
    if(std::regex_match(s, int_re))
        return ast_type_t::INT;
    if(std::regex_match(s, float_re) || std::regex_match(s, rational_re))
        return ast_type_t::FLOAT;
    return ast_type_t::NAME;
}

static bool is_delimiter(int c)
{
    return (c == '(' || c == ')' || c == ';' || c == '"' || c == '\'' || c == '`' ||
            c == '|' || c == '[' || c == ']' || c == '{' || c == '}' || c == EOF || isspace(c));
}

static int skip_whitespace(std::istream &is)
{
    int c;
    while(true)
    {
        c = is.peek();
        while(c != EOF && isspace(c))
        {
            is.get();
            c = is.peek();
        }
        if(c == ';')
        {
            while(c != EOF && c != '\n')
            {
                c = is.get();
            }
        }
        else
            return c;
    }
}

static ParseResult read_quoted(std::istream &is, const std::shared_ptr<ASTNode> &node, const std::string &form)
{
    auto quote = std::make_shared<ASTNode>();
    quote->type = ast_type_t::NAME;
    quote->value = form;
    node->type = ast_type_t::LIST;
    node->list.push_back(quote);
    auto object = read_object(is);
    if(object.result == parse_result_t::OK)
    {
        node->list.push_back(object.node);
        return node;
    }
    return object;
}

static ParseResult read_list(std::istream &is, const std::shared_ptr<ASTNode> &node)
{
    node->type = ast_type_t::LIST;
    skip_whitespace(is);
    int c = is.peek();
    while(c != EOF && c != ')')
    {
        auto object = read_object(is);
        if(object.result != parse_result_t::OK)
            return object;
        node->list.push_back(object.node);
        c = skip_whitespace(is);
    }
    if(c == EOF)
        return ParseResult("Unclosed list literal");
    is.get();
    return ParseResult(node);
}

static char parse_oct_char(const std::string &oct)
{
    return ((oct[0] - '0') << 6) + ((oct[1] - '0') << 3) + (oct[2] - '0');
}

ParseResult read_object(std::istream &is)
{
    auto o = std::make_shared<ASTNode>();
    skip_whitespace(is);
    int c = is.get();
    std::string charname;
    ParseResult result;
    switch(c)
    {
        case EOF:
            return ParseResult();
        case ')':
            return ParseResult("Invalid syntax");
        case '"':
            o->type = ast_type_t::STRING;
            c = is.get();
            while(c != EOF && c != '"')
            {
                if(c == '\\')
                {
                    c = is.get();
                    if(c == 'n')
                        o->value.push_back('\n');
                    else if(c == 't')
                        o->value.push_back('\t');
                    else if(c == 'f')
                        o->value.push_back('\f');
                    else if('0' <= c && c <= '7')
                    {
                        std::string oct;
                        for(int i = 0; i < 2; ++i)
                        {
                            oct.push_back(c);
                            c = is.get();
                            if(!('0' <= c && c <= '7'))
                                return ParseResult("Invalid escape sequence: \\" + oct + char(c));
                        }
                        oct.push_back(c);
                        o->value.push_back(parse_oct_char(oct));
                    }
                    else
                        o->value.push_back(c);
                }
                else
                    o->value.push_back(c);
                c = is.get();
            }
            if(c == EOF)
                return ParseResult("Unclosed string literal");
            return o;
        case '(':
            if((result = read_list(is, o)).result != parse_result_t::OK)
                return result;
            return o;
        case '\'':
            if((result = read_quoted(is, o, "quote")).result != parse_result_t::OK)
                return result;
            return o;
        case '`':
            if((result = read_quoted(is, o, "quasiquote")).result != parse_result_t::OK)
                return result;
            return o;
        case ',':
            if(is.peek() == '@')
            {
                is.get();
                if((result = read_quoted(is, o, "unquote-splicing")).result != parse_result_t::OK)
                    return result;
            }
            else
            {
                if((result = read_quoted(is, o, "unquote")).result != parse_result_t::OK)
                    return result;
            }
            return o;
        case '#':
            c = tolower(is.get());
            switch(c)
            {
                case 't':
                    o->type = ast_type_t::BOOL;
                    o->value = "t";
                    return o;
                case 'f':
                    o->type = ast_type_t::BOOL;
                    o->value = "f";
                    return o;
                case '\\':
                    o->type = ast_type_t::CHAR;
                    c = is.get();
                    if(c == EOF)
                        return ParseResult("Invalid character");
                    charname.clear();
                    charname.push_back(c);
                    c = is.peek();
                    while(!is_delimiter(c))
                    {
                        charname.push_back(c);
                        is.get();
                        c = is.peek();
                    }
                    o->value = normalize_char_name(charname);
                    if(o->value.empty())
                        return ParseResult("Invalid character: #\\" + charname);
                    return o;
                case '(':
                    if((result = read_list(is, o)).result != parse_result_t::OK)
                        return result;
                    o->type = ast_type_t::VECTOR;
                    return o;
                case '!':
                    o->value = "#!";
                    c = is.get();
                    break;
                default:
                    return ParseResult(std::string("Invalid sequence: #") + char(c));
            }
        default:
            o->value.push_back(tolower(c));
            c = is.peek();
            while(!is_delimiter(c))
            {
                o->value.push_back(tolower(c));
                is.get();
                c = is.peek();
            }
            o->type = identifier_type(o->value);
            return o;
    }
}