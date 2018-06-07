#include "eval.h"
#include "std.h"
#include <iostream>
#include <regex>

std::shared_ptr<SchemeObject> scheme_true = std::make_shared<SchemeBool>(true);
std::shared_ptr<SchemeObject> scheme_false = std::make_shared<SchemeBool>(false);
std::shared_ptr<SchemeObject> scheme_empty = std::make_shared<SchemeName>("");
std::shared_ptr<SchemeObject> scheme_nil = std::make_shared<SchemePair>(nullptr, nullptr);

static bool pair_function(const std::string &s)
{
    static const std::regex re("^c[ad]+r$");
    return std::regex_match(s, re);
}

static std::shared_ptr<SchemeObject> execute_pair_function(const std::string &name, std::shared_ptr<SchemeObject> p)
{
    for(size_t i = name.length() - 2; i; --i)
    {
        auto pp = std::dynamic_pointer_cast<SchemePair>(p);
        if(!pp || pp == scheme_nil)
            throw eval_error(name + ": not a pair");
        if(name[i] == 'a')
            p = pp->car;
        else
            p = pp->cdr;
    }
    return p;
}

std::shared_ptr<SchemeObject>
execute_function(std::shared_ptr<SchemeFunc> f, const std::list<std::shared_ptr<SchemeObject>> &val_list)
{
    std::shared_ptr<SchemeBuiltinFunc> bf = std::dynamic_pointer_cast<SchemeBuiltinFunc>(f);
    if(bf)
    {
        if(special_forms.count(bf->name))
            throw eval_error(bf->name + " cannot be executed this way");
        if(FunctionRegistry::exists(bf->name))
            return FunctionRegistry::get(bf->name)(val_list);
        if(val_list.size() != 1)
            throw eval_error(bf->name + ": one argument required");
        return execute_pair_function(bf->name, val_list.front());
    }
    Context local_context = f->context;
    local_context.newFrame();
    auto pit = f->params.begin();
    auto lit = val_list.begin();
    for(; lit != val_list.end() && pit != f->params.end(); ++lit, ++pit)
    {
        if(f->arglist && next(pit) == f->params.end())
        {
            auto rem = scheme_nil;
            for(auto rlit = val_list.rbegin(); rlit.base() != lit; ++rlit)
                rem = std::make_shared<SchemePair>(*rlit, rem);
            local_context.set(*pit, rem);
            lit = val_list.end();
            ++pit;
            break;
        }
        local_context.set(*pit, *lit);
    }
    if(f->arglist && next(pit) == f->params.end())
    {
        local_context.set(*pit, scheme_nil);
        ++pit;
    }
    if(lit != val_list.end())
    {
        throw eval_error("Too many arguments");
    }
    if(pit != f->params.end())
    {
        throw eval_error("Too few arguments");
    }
    std::shared_ptr<SchemeObject> res;
    for(auto i = f->body.begin(); i != f->body.end(); ++i)
    {
        res = i->evaluate(local_context, std::next(i) == f->body.end() ? f : nullptr);
    }
    return res;
}

std::shared_ptr<SchemeObject> ASTNode::evaluate(Context &context, std::shared_ptr<SchemeFunc> tail_func)
{
    if(type == ast_type_t::INT)
        return std::make_shared<SchemeInt>(stoll(value));
    if(type == ast_type_t::FLOAT)
        return std::make_shared<SchemeFloat>(stod(value));
    if(type == ast_type_t::STRING)
        return std::make_shared<SchemeString>(value);
    if(type == ast_type_t::BOOL)
        return std::make_shared<SchemeBool>(value == "t");
    if(type == ast_type_t::NAME)
    {
        auto t = context.get(value);
        if(t)
            return t;
        else if(special_forms.count(value) || FunctionRegistry::exists(value) || pair_function(value))
            return std::make_shared<SchemeBuiltinFunc>(value);
        else
            throw eval_error("Undefined name: " + value);
    }
    if(list.empty())
        throw eval_error("Trying to evaluate empty list");

    std::shared_ptr<SchemeFunc> f = std::dynamic_pointer_cast<SchemeFunc>(list.front()->evaluate(context));
    if(!f)
    {
        throw eval_error("Trying to call not a function");
    }
    std::shared_ptr<SchemeBuiltinFunc> bf = std::dynamic_pointer_cast<SchemeBuiltinFunc>(f);
    if(bf && special_forms.count(bf->name))
    {
        auto nl = list;
        nl.pop_front();
        return special_forms[bf->name](nl, context, tail_func);
    }
    std::list<std::shared_ptr<SchemeObject>> val_list;
    for(auto lit = std::next(list.begin()); lit != list.end(); ++lit)
    {
        val_list.push_back((*lit)->evaluate(context));
    }
    if(f == tail_func)
        throw tail_call(val_list);
    while(true)
    {
        try
        {
            return execute_function(f, val_list);
        }
        catch(tail_call &e)
        {
            val_list = e.list;
        }
    }
}

