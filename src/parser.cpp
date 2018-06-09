#include <regex>
#include <string>
#include "parser.h"
#include "char.h"

ast_type_t identifier_type(const std::string &s)
{
    static const std::regex float_re("^-?(\\d+\\.\\d*|\\d*\\.\\d+)$");
    static const std::regex int_re("^-?\\d+$");
    if(std::regex_match(s, int_re))
        return ast_type_t::INT;
    if(std::regex_match(s, float_re))
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

void readQuoted(std::istream &is, std::shared_ptr<ASTNode> node, const std::string &form)
{
    auto quote = std::make_shared<ASTNode>();
    quote->type = ast_type_t::NAME;
    quote->value = form;
    node->type = ast_type_t::LIST;
    node->list.push_back(quote);
    node->list.push_back(readObject(is));
}

static char parse_oct_char(const std::string &oct)
{
    return ((oct[0] - '0') << 6) + ((oct[1] - '0') << 3) + (oct[2] - '0');
}

std::shared_ptr<ASTNode> readObject(std::istream &is)
{
    auto o = std::make_shared<ASTNode>();
    skip_whitespace(is);
    int c = is.get();
    std::string charname;
    switch(c)
    {
        case EOF:
            throw end_of_input("");
        case ')':
            throw parse_error("Invalid syntax");
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
                        for(int i=0;i<2;++i)
                        {
                            oct.push_back(c);
                            c = is.get();
                            if(!('0' <= c && c <= '7'))
                                throw parse_error("Invalid escape sequence: \\" + oct + char(c));
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
                throw parse_error("Unclosed string literal");
            return o;
        case '(':
            o->type = ast_type_t::LIST;
            skip_whitespace(is);
            c = is.peek();
            while(c != EOF && c != ')')
            {
                o->list.push_back(readObject(is));
                c = skip_whitespace(is);
            }
            if(c == EOF)
                throw parse_error("Unclosed list literal");
            is.get();
            return o;
        case '\'':
            readQuoted(is, o, "quote");
            return o;
        case '`':
            readQuoted(is, o, "quasiquote");
            return o;
        case ',':
            if(is.peek() == '@')
            {
                is.get();
                readQuoted(is, o, "unquote-splicing");
            }
            else
            {
                readQuoted(is, o, "unquote");
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
                        throw parse_error("Invalid character");
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
                        throw parse_error("Invalid character: #\\" + charname);
                    return o;
                default:
                    throw parse_error(std::string("Invalid sequence: #") + char(c));
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