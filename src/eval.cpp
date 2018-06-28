#include "std.h"
#include <iostream>
#include <assert.h>
#include "char.h"

std::shared_ptr<SchemeObject> scheme_true = std::make_shared<SchemeBool>(true);
std::shared_ptr<SchemeObject> scheme_false = std::make_shared<SchemeBool>(false);
std::shared_ptr<SchemeObject> scheme_empty = std::make_shared<SchemeSymbol>("");
std::shared_ptr<SchemeObject> scheme_nil = std::make_shared<SchemePair>(nullptr, nullptr);


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
    if((long long) val_list.size() < f->arity.first)
        throw eval_error("Too few arguments");
    if(f->arity.second >= 0 && (long long) val_list.size() > f->arity.second)
        throw eval_error("Too many arguments");
    if(bf)
    {
        if(SpecialFormRegistry::exists(bf->name))
            throw eval_error(bf->name + " cannot be executed this way");
        if(FunctionRegistry::exists(bf->name))
            return std::get<2>(FunctionRegistry::get(bf->name))(val_list);
        assert(false);
    }

    Context local_context = f->context;
    local_context.new_frame();
    auto pit = f->params.begin();
    auto lit = val_list.begin();

    for(size_t i=0; i < std::min(val_list.size(), f->params.size() - (f->arity.second < 0)); ++lit, ++pit, ++i)
    {
        local_context.set(*pit, *lit);
    }
    if(lit == val_list.end())
    {
        for(;pit!=f->params.end(); ++pit)
        {
            local_context.set(*pit, scheme_empty);
        }
        if(f->arity.second < 0)
            local_context.set(f->params.back(), scheme_nil);
    }
    else
    {
        auto rem = scheme_nil;
        for(auto rlit = val_list.rbegin(); rlit.base() != lit; ++rlit)
            rem = std::make_shared<SchemePair>(*rlit, rem);
        local_context.set(f->params.back(), rem);
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
