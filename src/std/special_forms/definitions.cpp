#include <list>
#include <iostream>
#include "std.h"
#include "parser.h"


static std::shared_ptr<SchemeObject>
make_function(const std::string &name, const std::list<std::shared_ptr<ASTNode>> &l, const Context &context,
              const std::string &form)
{
    if(l.size() < 2 || l.front()->type != ast_type_t::LIST)
        throw eval_error(form + ": parameters and code required");
    auto pl = l.front()->list;
    if(form == "define" || form == "named-lambda")
        pl.pop_front();
    auto f = std::make_shared<SchemeCompoundProcedure>(name);
    int state = 0;
    for(const auto &i : pl)
    {
        if(i->type != ast_type_t::NAME)
            throw eval_error(form + ": list of names required");
        if(i->value == "#!optional")
        {
            if(state != 0)
                throw eval_error(form + ": invalid declaration");
            state = 1;
        }
        else if(i->value == "#!rest" || i->value == ".")
        {
            if(state > 1)
                throw eval_error(form + ": invalid declaration");
            state = 2;
            f->arity.second = -1;
        }
        else
        {
            f->params.push_back(i->value);
            if(state == 0)
            {
                ++f->arity.first;
                ++f->arity.second;
            }
            else if(state == 1)
            {
                ++f->arity.second;
            }
            else if(state == 2)
            {
                state = 3;
            }
            else
                throw eval_error(form + ": invalid declaration");
        }
    }
    if(state == 2)
        throw eval_error(form + ": invalid declaration");
    for(auto i = next(l.begin()); i != l.end(); ++i)
    {
        f->body.push_back(**i);
    }
    f->context = context;
    return f;
}


static SpecialFormPackage package(
    {
        {"define",       [](const std::list<std::shared_ptr<ASTNode>> &l, const Context &context) {
            if(l.size() < 1)
                throw eval_error("define: name and value required");
            if(l.front()->type == ast_type_t::NAME)
            {
                if(l.size() > 2)
                    throw eval_error("define: name and value required");
                auto res = l.size() == 2 ? l.back()->evaluate(context).force_value() : nullptr;
                context.set(l.front()->value, res);
                return ExecutionResult(res ? res : scheme_empty);
            }
            else if(l.front()->type == ast_type_t::LIST && l.front()->list.size() &&
                    l.front()->list.front()->type == ast_type_t::NAME)
            {
                auto f = make_function(l.front()->list.front()->value, l, context, "define");
                context.set(l.front()->list.front()->value, f);
                return ExecutionResult(f);
            }
            else
                throw eval_error("define: invalid arguments");
        }
        },
        {"lambda",       [](const std::list<std::shared_ptr<ASTNode>> &l, const Context &context) {
            if(l.size() > 0 && l.front()->type == ast_type_t::NAME)
            {
                auto ll = l;
                ll.pop_front();
                auto node = std::make_shared<ASTNode>();
                node->list.push_back(std::make_shared<ASTNode>(ast_type_t::NAME, "."));
                node->list.push_back(l.front());
                ll.push_front(node);
                return ExecutionResult(make_function("", ll, context, "lambda"));
            }
            return ExecutionResult(make_function("", l, context, "lambda"));
        }
        },
        {"named-lambda", [](const std::list<std::shared_ptr<ASTNode>> &l, const Context &context) {
            if(l.size() < 1 || l.front()->type != ast_type_t::LIST || l.front()->list.empty() ||
               l.front()->list.front()->type != ast_type_t::NAME)
                throw eval_error("named-lambda: name and code required");
            return ExecutionResult(make_function(l.front()->list.front()->value, l, context, "named-lambda"));
        }
        },
        {"let",          [](const std::list<std::shared_ptr<ASTNode>> &l, const Context &context) {
            if(l.size() < 2)
                throw eval_error("let: parameters and code required");
            std::list<std::shared_ptr<ASTNode>> pl;
            std::string name;
            if(l.front()->type == ast_type_t::NAME)
            {
                name = l.front()->value;
                pl = (*next(l.begin()))->list;
                if(l.size() < 3)
                    throw eval_error("let: parameters and code required");
            }
            else if(l.front()->type == ast_type_t::LIST)
                pl = l.front()->list;
            else
                throw eval_error("let: invalid argument");
            Context local_context = context;
            local_context.new_frame();
            for(auto i : pl)
            {
                if(i->type != ast_type_t::LIST || !(i->list.size() == 2 || i->list.size() == 1) ||
                   i->list.front()->type != ast_type_t::NAME)
                    throw eval_error("let: list of (name value) required");
                local_context.set(i->list.front()->value,
                                  i->list.size() == 2 ? i->list.back()->evaluate(context).force_value() : nullptr);
            }
            if(name.empty())
            {
                ExecutionResult res;
                for(auto i = next(l.begin()); i != l.end(); ++i)
                {
                    res.force_value();
                    res = (*i)->evaluate(local_context);
                }
                return res;
            }
            auto f = std::make_shared<SchemeCompoundProcedure>();
            f->context = local_context;
            for(auto i = next(next(l.begin())); i != l.end(); ++i)
            {
                f->body.push_back(**i);
            }
            f->arity.first = f->arity.second = pl.size();
            for(auto i : pl)
            {
                f->params.push_back(i->list.front()->value);
            }
            local_context.set(name, f);
            ExecutionResult res;
            for(auto i = f->body.begin(); i != f->body.end(); ++i)
            {
                res.force_value();
                res = i->evaluate(local_context);
            }
            return res;
        }
        },
        {"let*",         [](const std::list<std::shared_ptr<ASTNode>> &l, const Context &context) {
            if(l.size() < 2 || l.front()->type != ast_type_t::LIST)
                throw eval_error("let*: parameters and code required");
            auto pl = l.front()->list;
            Context local_context = context;
            for(auto i : pl)
            {
                if(i->type != ast_type_t::LIST || !(i->list.size() == 2 || i->list.size() == 1) ||
                   i->list.front()->type != ast_type_t::NAME)
                    throw eval_error("let*: list of (name value) required");
                auto value = i->list.size() == 2 ? i->list.back()->evaluate(local_context).force_value() : nullptr;
                local_context.new_frame();
                local_context.set(i->list.front()->value, value);
            }
            ExecutionResult res;
            for(auto i = next(l.begin()); i != l.end(); ++i)
            {
                res.force_value();
                res = (*i)->evaluate(local_context);
            }
            return res;
        }
        },
        {"letrec",       [](const std::list<std::shared_ptr<ASTNode>> &l, const Context &context) {
            if(l.size() < 2 || l.front()->type != ast_type_t::LIST)
                throw eval_error("letrec: parameters and code required");
            auto pl = l.front()->list;
            Context local_context = context;
            local_context.new_frame();
            for(auto i : pl)
            {
                if(i->type != ast_type_t::LIST || !(i->list.size() == 2 || i->list.size() == 1) ||
                   i->list.front()->type != ast_type_t::NAME)
                    throw eval_error("letrec: list of (name value) required");
                local_context.set(i->list.front()->value, nullptr);
            }
            for(auto i : pl)
            {
                auto value = i->list.size() == 2 ? i->list.back()->evaluate(local_context).force_value() : nullptr;
                local_context.set(i->list.front()->value, value);
            }
            ExecutionResult res;
            for(auto i = next(l.begin()); i != l.end(); ++i)
            {
                res.force_value();
                res = (*i)->evaluate(local_context);
            }
            return res;
        }
        },
        {"fluid-let",    [](const std::list<std::shared_ptr<ASTNode>> &l, const Context &context) {
            if(l.size() < 2 || l.front()->type != ast_type_t::LIST)
                throw eval_error("fluid-let: parameters and code required");
            auto pl = l.front()->list;
            std::map<std::string, std::shared_ptr<SchemeObject>> old_vars;
            for(auto i : pl)
            {
                if(i->type != ast_type_t::LIST || !(i->list.size() == 2 || i->list.size() == 1) ||
                   i->list.front()->type != ast_type_t::NAME)
                    throw eval_error("fluid-let: list of (name value) required");
                auto value = i->list.size() == 2 ? i->list.back()->evaluate(context).force_value() : nullptr;
                std::shared_ptr<SchemeObject> old{nullptr};
                if(!context.get(i->list.front()->value, old))
                    throw eval_error("fluid-let: unbound variable");
                old_vars[i->list.front()->value] = old;  // can be nullptr
                context.assign(i->list.front()->value, value);
            }
            std::shared_ptr<SchemeObject> res;
            for(auto i = next(l.begin()); i != l.end(); ++i)
            {
                res = (*i)->evaluate(context).force_value();
            }
            for(auto &&p : old_vars)
            {
                context.assign(p.first, p.second);
            }
            return ExecutionResult(res);
        }
        },
    }
);