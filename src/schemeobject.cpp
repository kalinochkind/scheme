#include <set>
#include "schemeobject.h"
#include "std.h"
#include "eval.h"


static std::string quote_string(const std::string &s)
{
    std::string ans = "\"";
    for(char c : s)
    {
        switch(c)
        {
            case '"':
            case '\\':
                ans.push_back('\\');
                ans.push_back(c);
                break;
            case '\n':
                ans.push_back('\\');
                ans.push_back('n');
                break;
            case '\t':
                ans.push_back('\\');
                ans.push_back('t');
                break;
            case '\f':
                ans.push_back('\\');
                ans.push_back('f');
                break;
            default:
                ans.push_back(c);
        }
    }
    return ans + "\"";
}

std::shared_ptr<ASTNode> SchemeObject::toAST() const
{
    throw eval_error("Cannot evaluate " + externalRepr());
}

std::string SchemeInt::externalRepr() const
{
    return std::to_string(value);
}

std::shared_ptr<ASTNode> SchemeInt::toAST() const
{
    return std::make_shared<ASTNode>(ast_type_t::INT, externalRepr());
}

std::string SchemeFloat::externalRepr() const
{
    return std::to_string(value);
}

std::shared_ptr<ASTNode> SchemeFloat::toAST() const
{
    return std::make_shared<ASTNode>(ast_type_t::FLOAT, externalRepr());
}

std::string SchemeFunc::externalRepr() const
{
    std::string res = "<function " + name + "(";
    for(auto i = params.begin(); i != params.end(); ++i)
    {
        if(i != params.begin())
            res += " ";
        if(arglist && next(i) == params.end())
            res += ". ";
        res += *i;
    }
    return res + ")>";
}

std::string SchemeBuiltinFunc::externalRepr() const
{
    if(special_forms.count(name))
        return "<special form '" + name + "'>";
    else
        return "<builtin function '" + name + "'>";
}

std::string SchemeBool::externalRepr() const
{
    return value ? "#t" : "#f";
}

std::shared_ptr<ASTNode> SchemeBool::toAST() const
{
    return std::make_shared<ASTNode>(ast_type_t::BOOL, value ? "t" : "f");
}

std::string SchemeString::externalRepr() const
{
    return quote_string(value);
}

std::shared_ptr<ASTNode> SchemeString::toAST() const
{
    return std::make_shared<ASTNode>(ast_type_t::STRING, value);
}

std::string SchemeString::printable() const
{
    return value;
}

std::string SchemePair::externalRepr() const
{
    if(!car)
        return "()";
    auto pcar = std::dynamic_pointer_cast<SchemeName>(car);
    if(pcar)
    {
        std::string prefix = "";
        if(pcar->value == "quote")
            prefix = "'";
        else if(pcar->value == "quasiquote")
            prefix = "`";
        else if(pcar->value == "unquote")
            prefix = ",";
        else if(pcar->value == "unquote-splicing")
            prefix = ",@";
        if(prefix.length())
        {
            auto pcdr = std::dynamic_pointer_cast<SchemePair>(cdr);
            if(pcdr->cdr == scheme_nil)
                return prefix + pcdr->car->externalRepr();
        }
    }
    std::string res = "(" + car->externalRepr();
    std::shared_ptr<SchemeObject> p = cdr;
    std::shared_ptr<SchemePair> pp;
    std::set<const SchemePair *> visited{this};
    while((pp = std::dynamic_pointer_cast<SchemePair>(p)))
    {
        if(p == scheme_nil)
            return res + ")";
        res += " " + pp->car->externalRepr();
        if(visited.count(pp.get()))
            return res + " ...)";
        p = pp->cdr;
        visited.insert(pp.get());
    }
    return res + " . " + p->externalRepr() + ")";
}

std::shared_ptr<ASTNode> SchemePair::toAST() const
{
    auto a = std::make_shared<ASTNode>();
    const SchemePair *p(this);
    while(p && p != scheme_nil.get())
    {
        a->list.push_back(p->car->toAST());
        p = dynamic_cast<const SchemePair*>(p->cdr.get());
    }
    if(!p)
    {
        return SchemeObject::toAST();
    }
    return a;
}

std::string SchemeName::externalRepr() const
{
    return value;
}

std::shared_ptr<ASTNode> SchemeName::toAST() const
{
    return std::make_shared<ASTNode>(ast_type_t::NAME, value);
}

std::string SchemePromise::externalRepr() const
{
    return "<promise>";
}

std::shared_ptr<SchemeObject> SchemePromise::force()
{
    if(!value)
    {
        value = execute_function(func, {});
        func.reset();
    }
    return value;
}

std::string SchemeEnvironment::externalRepr() const
{
    return "<environment>";
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

void Context::set(const std::string &name, std::shared_ptr<SchemeObject> value)
{
    (*(locals.back()))[name] = value;
}

void Context::assign(const std::string &name, std::shared_ptr<SchemeObject> value)
{
    for(auto i = locals.rbegin(); i != locals.rend(); ++i)
    {
        if((*i)->count(name))
        {
            (**i)[name] = value;
            return;
        }
    }
    (*locals.front())[name] = value;
}

void Context::newFrame()
{
    locals.push_back(std::make_shared<context_map_t>());
}

void Context::newFrame(const context_map_t &vars)
{
    locals.push_back(std::make_shared<context_map_t>(vars));
}

void Context::delFrame()
{
    locals.pop_back();
}