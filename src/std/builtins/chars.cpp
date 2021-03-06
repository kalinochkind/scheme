#include "std.h"
#include "char.h"

static BuiltinFunction char_predicate(const std::string &name, int (*fun)(int))
{
    return {1, 1, [name, fun](const std::list<std::shared_ptr<SchemeObject>> &l) {
        auto p = std::dynamic_pointer_cast<SchemeChar>(l.front());
        if(!p)
            throw eval_error(name + ": a char required");
        return fun(p->value) ? scheme_true : scheme_false;
    }};
}

static FunctionPackage package(
    {
        {"char->name", {1, 2, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            auto p = std::dynamic_pointer_cast<SchemeChar>(l.front());
            if(!p)
                throw eval_error("char->name: a char required");
            return to_object(std::make_shared<SchemeString>(char_to_char_name(p->value)));
        }}},
        {"name->char", {1, 1, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            auto p = std::dynamic_pointer_cast<SchemeString>(l.front());
            if(!p)
                throw eval_error("name->char: a string required");
            if(normalize_char_name(p->value).empty())
                throw eval_error("name->char: invalid name: '" + p->value + "'");
            return to_object(std::make_shared<SchemeChar>(char_name_to_char(p->value)));
        }}},
        {"char-upcase", {1, 1, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            auto p = std::dynamic_pointer_cast<SchemeChar>(l.front());
            if(!p)
                throw eval_error("char-upcase: a char required");
            return to_object(std::make_shared<SchemeChar>(toupper(p->value)));
        }}},
        {"char-downcase", {1, 1, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            auto p = std::dynamic_pointer_cast<SchemeChar>(l.front());
            if(!p)
                throw eval_error("char-downcase: a char required");
            return to_object(std::make_shared<SchemeChar>(tolower(p->value)));
        }}},
        {"char->integer", {1, 1, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            auto p = std::dynamic_pointer_cast<SchemeChar>(l.front());
            if(!p)
                throw eval_error("char->integer: a char required");
            return to_object(std::make_shared<SchemeInt>(p->value));
        }}},
        {"integer->char", {1, 1, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            auto p = std::dynamic_pointer_cast<SchemeInt>(l.front());
            if(!p)
                throw eval_error("integer->char: an integer required");
            return to_object(std::make_shared<SchemeChar>(p->value & 0x7f));

        }}},
        {"char-upper-case?", char_predicate("char-upper-case?", isupper)},
        {"char-lower-case?", char_predicate("char-lower-case?", islower)},
        {"char-alphabetic?", char_predicate("char-alphabetic?", isalpha)},
        {"char-numeric?", char_predicate("char-numeric?", isdigit)},
        {"char-alphanumeric?", char_predicate("char-alphanumeric?", isalnum)},
    }
);