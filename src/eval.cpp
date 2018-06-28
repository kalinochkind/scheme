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
        (*this) = tail_context->func->execute(tail_context->args);
    }
    return value;
}

ExecutionResult SchemeProcedure::execute(const std::list<std::shared_ptr<SchemeObject>> &val_list) const
{
    if((long long) val_list.size() < arity.first)
        throw eval_error("Too few arguments");
    if(arity.second >= 0 && (long long) val_list.size() > arity.second)
        throw eval_error("Too many arguments");
    return _run(val_list);
}


ExecutionResult SchemePrimitiveProcedure::_run(const std::list<std::shared_ptr<SchemeObject>> &val_list) const
{
    return std::get<2>(FunctionRegistry::get(name))(val_list);
}

ExecutionResult SchemeCompoundProcedure::_run(const std::list<std::shared_ptr<SchemeObject>> &val_list) const
{
    Context local_context = context;
    local_context.new_frame();
    auto pit = params.begin();
    auto lit = val_list.begin();

    for(size_t i=0; i < std::min(val_list.size(), params.size() - (arity.second < 0)); ++lit, ++pit, ++i)
    {
        local_context.set(*pit, *lit);
    }
    if(lit == val_list.end())
    {
        for(;pit!=params.end(); ++pit)
        {
            local_context.set(*pit, scheme_empty);
        }
        if(arity.second < 0)
            local_context.set(params.back(), scheme_nil);
    }
    else
    {
        auto rem = scheme_nil;
        for(auto rlit = val_list.rbegin(); rlit.base() != lit; ++rlit)
            rem = std::make_shared<SchemePair>(*rlit, rem);
        local_context.set(params.back(), rem);
    }

    ExecutionResult res;
    for(auto i = body.begin(); i != body.end(); ++i)
    {
        res.force_value();
        res = i->evaluate(local_context);
    }
    return res;
}

ExecutionResult SchemeSpecialForm::execute(std::list<std::shared_ptr<ASTNode>> l, Context &context)
{
    l.pop_front();
    return SpecialFormRegistry::get(name)(l, context);
}


ExecutionResult ASTNode::evaluate(Context &context) const
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

    auto func = list.front()->evaluate(context).force_value();
    auto sf = std::dynamic_pointer_cast<SchemeSpecialForm>(func);
    if(sf)
    {
        return sf->execute(list, context);
    }
    auto f = std::dynamic_pointer_cast<SchemeProcedure>(func);
    if(!f)
    {
        throw eval_error("Trying to call not a function");
    }
    std::list<std::shared_ptr<SchemeObject>> val_list;
    for(auto lit = std::next(list.begin()); lit != list.end(); ++lit)
    {
        val_list.push_back((*lit)->evaluate(context).force_value());
    }
    return ExecutionResult(f, val_list);
}
