#include "std.h"
#include <iostream>
#include <regex>
#include "char.h"

std::shared_ptr<SchemeObject> scheme_true = std::make_shared<SchemeBool>(true);
std::shared_ptr<SchemeObject> scheme_false = std::make_shared<SchemeBool>(false);
std::shared_ptr<SchemeObject> scheme_empty = std::make_shared<SchemeSymbol>("");
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

std::shared_ptr<SchemeObject> ExecutionResult::force_value()
{
    while(type == execution_result_t::TAIL_CALL)
    {
        (*this) = execute_function(tail_context->func, tail_context->args);
    }
    return value;
}

ExecutionResult
execute_function(std::shared_ptr<SchemeFunc> f, const std::list<std::shared_ptr<SchemeObject>> &val_list)
{
    std::shared_ptr<SchemeBuiltinFunc> bf = std::dynamic_pointer_cast<SchemeBuiltinFunc>(f);
    if(bf)
    {
        if(SpecialFormRegistry::exists(bf->name))
            throw eval_error(bf->name + " cannot be executed this way");
        if(FunctionRegistry::exists(bf->name))
            return FunctionRegistry::get(bf->name)(val_list);
        if(val_list.size() != 1)
            throw eval_error(bf->name + ": one argument required");
        return ExecutionResult(execute_pair_function(bf->name, val_list.front()));
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
    ExecutionResult res;
    for(auto i = f->body.begin(); i != f->body.end(); ++i)
    {
        res.force_value();
        res = i->evaluate(local_context);
    }
    return res;
}

ExecutionResult ASTNode::evaluate(Context &context)
{

    if(type == ast_type_t::INT)
        try
        {
            return ExecutionResult(std::make_shared<SchemeInt>(stoll(value)));
        }
        catch(std::out_of_range &)
        {
            throw eval_error("Invalid integer: " + value);
        }
    if(type == ast_type_t::FLOAT)
        try
        {
            return ExecutionResult(std::make_shared<SchemeFloat>(stod(value)));
        }
        catch(std::out_of_range &)
        {
            throw eval_error("Invalid float: " + value);
        }
    if(type == ast_type_t::STRING)
        return ExecutionResult(std::make_shared<SchemeString>(value));
    if(type == ast_type_t::CHAR)
        return ExecutionResult(std::make_shared<SchemeChar>(char_name_to_char(value)));
    if(type == ast_type_t::BOOL)
        return ExecutionResult(value == "t" ? scheme_true : scheme_false);
    if(type == ast_type_t::NAME)
    {
        std::shared_ptr<SchemeObject> t{nullptr};
        bool res = context.get(value, t);
        if(t)
            return ExecutionResult(t);
        else if(res)
            throw eval_error("Unassigned variable: " + value);
        else if(pair_function(value))
            return ExecutionResult(std::make_shared<SchemeBuiltinFunc>(value));
        else
            throw eval_error("Undefined name: " + value);
    }
    if(type == ast_type_t::VECTOR)
        throw eval_error("Trying to evaluate a vector");
    if(list.empty())
        throw eval_error("Trying to evaluate empty list");

    auto f = std::dynamic_pointer_cast<SchemeFunc>(list.front()->evaluate(context).force_value());
    if(!f)
    {
        throw eval_error("Trying to call not a function");
    }
    auto bf = std::dynamic_pointer_cast<SchemeBuiltinFunc>(f);
    if(bf && SpecialFormRegistry::exists(bf->name))
    {
        auto nl = list;
        nl.pop_front();
        return SpecialFormRegistry::get(bf->name)(nl, context);
    }
    std::list<std::shared_ptr<SchemeObject>> val_list;
    for(auto lit = std::next(list.begin()); lit != list.end(); ++lit)
    {
        val_list.push_back((*lit)->evaluate(context).force_value());
    }
    return ExecutionResult(f, val_list);
}
