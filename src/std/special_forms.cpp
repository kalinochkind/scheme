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

static std::shared_ptr<SchemeObject> make_promise(std::shared_ptr<ASTNode> body, Context &context)
{
    auto f = std::make_shared<SchemeFunc>();
    f->context = context;
    f->body = {*body};
    return std::dynamic_pointer_cast<SchemeObject>(std::make_shared<SchemePromise>(f));
}

std::unordered_map<std::string, std::function<std::shared_ptr<SchemeObject>(const std::list<std::shared_ptr<ASTNode>> &,
                                                                            Context &context,
                                                                            std::shared_ptr<SchemeFunc> tail_func)>> special_forms = {
        {"define",          [](const std::list<std::shared_ptr<ASTNode>> &l, Context &context,
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
        {"lambda",          [](const std::list<std::shared_ptr<ASTNode>> &l, Context &context,
                               std::shared_ptr<SchemeFunc>) {
            return make_function("", l, context, "lambda");
        }
        },
        {"named-lambda",    [](const std::list<std::shared_ptr<ASTNode>> &l, Context &context,
                               std::shared_ptr<SchemeFunc>) {
            if(l.size() < 1 || l.front()->type != ast_type_t::LIST || l.front()->list.empty() ||
               l.front()->list.front()->type != ast_type_t::NAME)
                throw eval_error("named-lambda: name and code required");
            return make_function(l.front()->list.front()->value, l, context, "named-lambda");
        }
        },
        {"let",             [](const std::list<std::shared_ptr<ASTNode>> &l, Context &context,
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
        {"let*",            [](const std::list<std::shared_ptr<ASTNode>> &l, Context &context,
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
        {"letrec",          [](const std::list<std::shared_ptr<ASTNode>> &l, Context &context,
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
        {"fluid-let",       [](const std::list<std::shared_ptr<ASTNode>> &l, Context &context,
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
        {"do",              [](const std::list<std::shared_ptr<ASTNode>> &l, Context &context,
                               std::shared_ptr<SchemeFunc> tail_func) {
            if(l.size() < 2 || l.front()->type != ast_type_t::LIST || (*next(l.begin()))->type != ast_type_t::LIST)
                throw eval_error("do: lists required");
            context_map_t new_frame;
            auto vars = l.front()->list;
            auto cond = *next(l.begin());
            if(cond->list.empty())
                throw eval_error("do: empty condition");
            for(auto &&i : vars)
            {
                if(i->type != ast_type_t::LIST || i->list.empty() || i->list.size() > 3 ||
                   i->list.front()->type != ast_type_t::NAME)
                    throw eval_error("do: bad variable list");
                if(new_frame.count(i->list.front()->value))
                    throw eval_error("do: duplicate variables");
                new_frame[i->list.front()->value] = i->list.empty() ? nullptr : (*next(i->list.begin()))->evaluate(
                        context);
            }
            Context local_context = context;
            local_context.newFrame(new_frame);
            while(true)
            {
                auto cond_value = cond->list.front()->evaluate(local_context);
                if(cond_value->toBool())
                {
                    for(auto it = next(cond->list.begin()); it != cond->list.end(); ++it)
                    {
                        cond_value = (*it)->evaluate(local_context, next(it) == cond->list.end() ? tail_func : nullptr);
                    }
                    return cond_value;
                }
                for(auto it = next(next(l.begin())); it != l.end(); ++it)
                {
                    (*it)->evaluate(local_context);
                }
                new_frame.clear();
                for(auto &&i : vars)
                {
                    if(i->list.size() < 3)
                        new_frame[i->list.front()->value] = local_context.get(i->list.front()->value);
                    else
                        new_frame[i->list.front()->value] = i->list.back()->evaluate(local_context);
                }
                local_context.delFrame();
                local_context.newFrame(new_frame);
            }
        }
        },
        {"cond",            [](const std::list<std::shared_ptr<ASTNode>> &l, Context &context,
                               std::shared_ptr<SchemeFunc> tail_func) {
            for(auto branch : l)
            {
                if(branch->type != ast_type_t::LIST || branch->list.size() < 1)
                    throw eval_error("cond: non-empty lists required");
                std::shared_ptr<SchemeObject> br;
                if((branch->list.front()->type == ast_type_t::NAME && branch->list.front()->value == "else") ||
                   (br = branch->list.front()->evaluate(context))->toBool())
                {
                    if(branch->list.size() == 1)
                    {
                        if(branch->list.front()->type == ast_type_t::NAME && branch->list.front()->value == "else")
                            throw eval_error("cond: else branch has no expressions");
                        return br;
                    }
                    if(branch->list.size() == 3 && ((*next(branch->list.begin()))->type == ast_type_t::NAME &&
                                                    (*next(branch->list.begin()))->value == "=>"))
                    {
                        auto func = std::dynamic_pointer_cast<SchemeFunc>(branch->list.back()->evaluate(context));
                        if(!func)
                            throw eval_error("cond: => requires a function");
                        std::list<std::shared_ptr<SchemeObject>> args{br};
                        if(func == tail_func)
                            throw tail_call(args);
                        return execute_function(func, args);
                    }
                    std::shared_ptr<SchemeObject> res;
                    for(auto i = next(branch->list.begin()); i != branch->list.end(); ++i)
                    {
                        res = (*i)->evaluate(context, std::next(i) == branch->list.end() ? tail_func : nullptr);
                    }
                    return res;
                }
            }
            return scheme_empty;
        }
        },
        {"if",              [](const std::list<std::shared_ptr<ASTNode>> &l, Context &context,
                               std::shared_ptr<SchemeFunc> tail_func) {
            if(l.size() > 3 || l.size() < 2)
                throw eval_error("if: 2 or 3 arguments required");
            if(l.front()->evaluate(context)->toBool())
                return (*next(l.begin()))->evaluate(context, tail_func);
            else if(l.size() == 3)
                return (*next(next(l.begin())))->evaluate(context, tail_func);
            else
                return scheme_empty;
        }
        },
        {"case",            [](const std::list<std::shared_ptr<ASTNode>> &l, Context &context,
                               std::shared_ptr<SchemeFunc> tail_func) {
            if(l.empty())
                throw eval_error("case: argument required");
            auto value = l.front()->evaluate(context);
            for(auto it = next(l.begin()); it != l.end(); ++it)
            {
                bool passed = false;
                if((*it)->type != ast_type_t::LIST || (*it)->list.empty())
                    throw eval_error("case: non-empty lists required");
                auto values = (*it)->list.front();
                if(values->type == ast_type_t::NAME && values->value == "else")
                    passed = true;
                else if(values->type != ast_type_t::LIST)
                {
                    throw eval_error("case: lists required");
                }
                else
                {
                    for(auto &&j : values->list)
                    {
                        if(eq_test(value, do_quote(j, context, 0).first))
                        {
                            passed = true;
                            break;
                        }
                    }
                }
                if(passed)
                {
                    auto result = scheme_empty;
                    for(auto j = next((*it)->list.begin()); j != (*it)->list.end(); ++j)
                    {
                        result = (*j)->evaluate(context, next(j) == (*it)->list.end() ? tail_func : nullptr);
                    }
                    return result;
                }
            }
            return scheme_empty;
        }
        },
        {"and",             [](const std::list<std::shared_ptr<ASTNode>> &l, Context &context,
                               std::shared_ptr<SchemeFunc> tail_func) {
            if(l.empty())
                return scheme_true;
            std::shared_ptr<SchemeObject> res;
            for(auto i = l.begin(); i != l.end(); ++i)
            {
                if(!(res = (*i)->evaluate(context, next(i) == l.end() ? tail_func : nullptr))->toBool())
                    return scheme_false;
            }
            return res;
        }
        },
        {"or",              [](const std::list<std::shared_ptr<ASTNode>> &l, Context &context,
                               std::shared_ptr<SchemeFunc> tail_func) {
            std::shared_ptr<SchemeObject> res;
            for(auto i = l.begin(); i != l.end(); ++i)
            {
                if((res = (*i)->evaluate(context, next(i) == l.end() ? tail_func : nullptr))->toBool())
                    return res;
            }
            return scheme_false;
        }
        },
        {"apply",           [](const std::list<std::shared_ptr<ASTNode>> &l, Context &context,
                               std::shared_ptr<SchemeFunc> tail_func) {
            if(l.size() < 2)
                throw eval_error("apply: function and list of arguments required");
            auto f = std::dynamic_pointer_cast<SchemeFunc>(l.front()->evaluate(context));
            auto tail = std::dynamic_pointer_cast<SchemePair>(l.back()->evaluate(context));
            if(!f || !tail)
                throw eval_error("apply: function and list of arguments required");
            std::list<std::shared_ptr<SchemeObject>> args;
            for(auto i = next(l.begin()); next(i) != l.end(); ++i)
                args.push_back((*i)->evaluate(context));
            while(tail && tail != scheme_nil)
            {
                args.push_back(tail->car);
                tail = std::dynamic_pointer_cast<SchemePair>(tail->cdr);
            }
            if(f == tail_func)
                throw tail_call(args);
            return execute_function(f, args);
        }
        },
        {"quote",           [](const std::list<std::shared_ptr<ASTNode>> &l, Context &context,
                               std::shared_ptr<SchemeFunc>) {
            if(l.size() != 1)
                throw eval_error("quote: one argument required");
            return do_quote(l.front(), context, 0).first;
        }
        },
        {"quasiquote",      [](const std::list<std::shared_ptr<ASTNode>> &l, Context &context,
                               std::shared_ptr<SchemeFunc>) {
            if(l.size() != 1)
                throw eval_error("quasiquote: one argument required");
            return do_quote(l.front(), context, 1).first;
        }
        },
        {"set!",            [](const std::list<std::shared_ptr<ASTNode>> &l, Context &context,
                               std::shared_ptr<SchemeFunc>) {
            if(!(l.size() == 1 || l.size() == 2) || l.front()->type != ast_type_t::NAME)
                throw eval_error("set!: name and value required");
            auto res = l.size() == 2 ? l.back()->evaluate(context) : nullptr;
            context.assign(l.front()->value, res);
            return res ? res : scheme_empty;
        }
        },
        {"begin",           [](const std::list<std::shared_ptr<ASTNode>> &l, Context &context,
                               std::shared_ptr<SchemeFunc> tail_func) {
            if(!l.size())
                throw eval_error("begin: at least one argument required");
            std::shared_ptr<SchemeObject> res;
            for(auto i = l.begin(); i != l.end(); ++i)
            {
                res = (*i)->evaluate(context, next(i) == l.end() ? tail_func : nullptr);
            }
            return res;
        }
        },
        {"delay",           [](const std::list<std::shared_ptr<ASTNode>> &l, Context &context,
                               std::shared_ptr<SchemeFunc>) {
            if(l.size() != 1)
                throw eval_error("delay: one argument required");
            return make_promise(l.front(), context);
        }
        },
        {"cons-stream",     [](const std::list<std::shared_ptr<ASTNode>> &l, Context &context,
                               std::shared_ptr<SchemeFunc>) {
            if(l.size() != 2)
                throw eval_error("cons-stream: two arguments required");
            return std::dynamic_pointer_cast<SchemeObject>(
                    std::make_shared<SchemePair>(l.front()->evaluate(context), make_promise(l.back(), context)));
        }
        },
        {"the-environment", [](const std::list<std::shared_ptr<ASTNode>> &, Context &context,
                               std::shared_ptr<SchemeFunc>) {
            return std::dynamic_pointer_cast<SchemeObject>(std::make_shared<SchemeEnvironment>(context));
        }
        },
};