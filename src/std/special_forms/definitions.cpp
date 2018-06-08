#include <list>
#include <iostream>
#include "std.h"
#include "eval.h"
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
    std::shared_ptr<SchemeFunc> f = std::make_shared<SchemeFunc>(name);
    for(auto i = pl.begin(); i != pl.end(); ++i)
    {
        if((*i)->type != ast_type_t::NAME)
            throw eval_error(form + ": list of names required");
        if((*i)->value == ".")
        {
            if(next(i) == pl.end() || next(next(i)) != pl.end())
                throw eval_error(form + ": incorrect dot syntax");
            f->arglist = true;
            continue;
        }
        f->params.push_back((*i)->value);
    }
    for(auto i = next(l.begin()); i != l.end(); ++i)
    {
        f->body.push_back(**i);
    }
    f->context = context;
    return std::dynamic_pointer_cast<SchemeObject>(f);
}


static Package package(
    {
        {"define",       [](const std::list<std::shared_ptr<ASTNode>> &l, Context &context,
                            std::shared_ptr<SchemeFunc>) {
            if(l.size() < 1)
                throw eval_error("define: name and value required");
            if(l.front()->type == ast_type_t::NAME)
            {
                if(l.size() > 2)
                    throw eval_error("define: name and value required");
                auto res = l.size() == 2 ? l.back()->evaluate(context) : nullptr;
                context.set(l.front()->value, res);
                return res ? res : scheme_empty;
            }
            else if(l.front()->type == ast_type_t::LIST && l.front()->list.size() &&
                    l.front()->list.front()->type == ast_type_t::NAME)
            {
                auto f = make_function(l.front()->list.front()->value, l, context, "define");
                context.set(l.front()->list.front()->value, f);
                return f;
            }
            else
                throw eval_error("define: invalid arguments");
        }
        },
        {"lambda",       [](const std::list<std::shared_ptr<ASTNode>> &l, Context &context,
                            std::shared_ptr<SchemeFunc>) {
            return make_function("", l, context, "lambda");
        }
        },
        {"named-lambda", [](const std::list<std::shared_ptr<ASTNode>> &l, Context &context,
                            std::shared_ptr<SchemeFunc>) {
            if(l.size() < 1 || l.front()->type != ast_type_t::LIST || l.front()->list.empty() ||
               l.front()->list.front()->type != ast_type_t::NAME)
                throw eval_error("named-lambda: name and code required");
            return make_function(l.front()->list.front()->value, l, context, "named-lambda");
        }
        },
        {"let",          [](const std::list<std::shared_ptr<ASTNode>> &l, Context &context,
                            std::shared_ptr<SchemeFunc> tail_func) {
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
            local_context.newFrame();
            for(auto i : pl)
            {
                if(i->type != ast_type_t::LIST || !(i->list.size() == 2 || i->list.size() == 1) ||
                   i->list.front()->type != ast_type_t::NAME)
                    throw eval_error("let: list of (name value) required");
                local_context.set(i->list.front()->value,
                                  i->list.size() == 2 ? i->list.back()->evaluate(context) : nullptr);
            }
            if(name.empty())
            {
                std::shared_ptr<SchemeObject> res;
                for(auto i = next(l.begin()); i != l.end(); ++i)
                {
                    res = (*i)->evaluate(local_context, next(i) == l.end() ? tail_func : nullptr);
                }
                return res;
            }
            auto f = std::make_shared<SchemeFunc>();
            f->context = local_context;
            for(auto i = next(next(l.begin())); i != l.end(); ++i)
            {
                f->body.push_back(**i);
            }
            for(auto i : pl)
            {
                f->params.push_back(i->list.front()->value);
            }
            local_context.set(name, f);
            std::shared_ptr<SchemeObject> res;
            for(auto i = f->body.begin(); i != f->body.end(); ++i)
            {
                res = i->evaluate(local_context, std::next(i) == f->body.end() ? tail_func : nullptr);
            }
            return res;
        }
        },
        {"let*",         [](const std::list<std::shared_ptr<ASTNode>> &l, Context &context,
                            std::shared_ptr<SchemeFunc> tail_func) {
            if(l.size() < 2 || l.front()->type != ast_type_t::LIST)
                throw eval_error("let*: parameters and code required");
            auto pl = l.front()->list;
            Context local_context = context;
            for(auto i : pl)
            {
                if(i->type != ast_type_t::LIST || !(i->list.size() == 2 || i->list.size() == 1) ||
                   i->list.front()->type != ast_type_t::NAME)
                    throw eval_error("let*: list of (name value) required");
                auto value = i->list.size() == 2 ? i->list.back()->evaluate(local_context) : nullptr;
                local_context.newFrame();
                local_context.set(i->list.front()->value, value);
            }
            std::shared_ptr<SchemeObject> res;
            for(auto i = next(l.begin()); i != l.end(); ++i)
            {
                res = (*i)->evaluate(local_context, next(i) == l.end() ? tail_func : nullptr);
            }
            return res;
        }
        },
        {"letrec",       [](const std::list<std::shared_ptr<ASTNode>> &l, Context &context,
                            std::shared_ptr<SchemeFunc> tail_func) {
            if(l.size() < 2 || l.front()->type != ast_type_t::LIST)
                throw eval_error("letrec: parameters and code required");
            auto pl = l.front()->list;
            Context local_context = context;
            local_context.newFrame();
            for(auto i : pl)
            {
                if(i->type != ast_type_t::LIST || !(i->list.size() == 2 || i->list.size() == 1) ||
                   i->list.front()->type != ast_type_t::NAME)
                    throw eval_error("letrec: list of (name value) required");
                local_context.set(i->list.front()->value, nullptr);
            }
            for(auto i : pl)
            {
                auto value = i->list.size() == 2 ? i->list.back()->evaluate(local_context) : nullptr;
                local_context.assign(i->list.front()->value, value);
            }
            std::shared_ptr<SchemeObject> res;
            for(auto i = next(l.begin()); i != l.end(); ++i)
            {
                res = (*i)->evaluate(local_context, next(i) == l.end() ? tail_func : nullptr);
            }
            return res;
        }
        },
        {"fluid-let",    [](const std::list<std::shared_ptr<ASTNode>> &l, Context &context,
                            std::shared_ptr<SchemeFunc>) {
            if(l.size() < 2 || l.front()->type != ast_type_t::LIST)
                throw eval_error("fluid-let: parameters and code required");
            auto pl = l.front()->list;
            std::map<std::string, std::shared_ptr<SchemeObject>> old_vars;
            for(auto i : pl)
            {
                if(i->type != ast_type_t::LIST || !(i->list.size() == 2 || i->list.size() == 1) ||
                   i->list.front()->type != ast_type_t::NAME)
                    throw eval_error("fluid-let: list of (name value) required");
                auto value = i->list.size() == 2 ? i->list.back()->evaluate(context) : nullptr;
                auto old = context.get(i->list.front()->value);
                old_vars[i->list.front()->value] = old;  // can be nullptr
                context.assign(i->list.front()->value, value);
            }
            std::shared_ptr<SchemeObject> res;
            for(auto i = next(l.begin()); i != l.end(); ++i)
            {
                res = (*i)->evaluate(context, nullptr);
            }
            for(auto &&p : old_vars)
            {
                context.assign(p.first, p.second);
            }
            return res;
        }
        },
    }
);