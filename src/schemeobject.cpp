#include <std.h>
#include "schemeobject.h"

std::string SchemeInt::toString() const
{
    return std::to_string(value);
}

std::string SchemeFloat::toString() const
{
    return std::to_string(value);
}

std::string SchemeFunc::toString() const
{
    std::string res = "<function (";
    for(auto i = params.begin(); i != params.end(); ++i)
    {
        if(i != params.begin())
            res += " ";
        res += *i;
    }
    res += ")";
    /* for(auto &&i : body)
     {
         res += " " + i.toString();
     }*/
    res += ">";
    return res;
}

std::string SchemeBuiltinFunc::toString() const
{
    if(special_forms.count(name))
        return "<special form '" + name + "'>";
    else
        return "<builtin function '" + name + "'>";
}

std::string SchemeBool::toString() const
{
    return value ? "#t" : "#f";
}

std::string SchemeString::toString() const
{
    return value;
}

std::string ASTNode::toString() const
{
    if(type != ast_type_t::LIST)
        return value;
    std::string res = "(";
    for(auto i = list.begin(); i != list.end(); ++i)
    {
        if(i != list.begin())
            res += " ";
        res += (*i)->toString();
    }
    res += ")";
    return res;
}

std::shared_ptr<SchemeObject> Context::get(const std::string &name) const
{
    for(auto i = locals.rbegin(); i != locals.rend(); ++i)
    {
        if((*i)->count(name))
            return (**i)[name];
    }
    return nullptr;
}

void Context::set(const std::string name, std::shared_ptr<SchemeObject> value)
{
    (*(locals.back()))[name] = value;
}

void Context::newFrame()
{
    locals.push_back(std::make_shared<context_map_t>());
}

void Context::delFrame()
{
    locals.pop_back();
}