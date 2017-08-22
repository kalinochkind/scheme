#include <list>
#include "std.h"
#include "eval.h"

static std::shared_ptr<SchemeObject>
make_function(const std::list<std::shared_ptr<ASTNode>> &l, const Context &context, const std::string &form)
{
    if(l.size() < 2 || l.front()->type != ast_type_t::LIST)
        throw eval_error(form + ": parameters and code required");
    auto pl = l.front()->list;
    if(form == "define")
        pl.pop_front();
    std::shared_ptr<SchemeFunc> f = std::make_shared<SchemeFunc>();
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

std::unordered_map<std::string, std::function<std::shared_ptr<SchemeObject>(const std::list<std::shared_ptr<ASTNode>> &,
                                                                            Context &context,
                                                                            std::shared_ptr<SchemeFunc> tail_func)>> special_forms = {
        {"define", [](const std::list<std::shared_ptr<ASTNode>> &l, Context &context,
                      std::shared_ptr<SchemeFunc>) {
            if(l.size() < 2)
                throw eval_error("define: at least 2 arguments required");
            if(l.front()->type == ast_type_t::NAME)
            {
                if(l.size() != 2)
                    throw eval_error("define: name and value required");
                auto res = (*next(l.begin()))->evaluate(context);
                context.set((*l.begin())->value, res);
                return res;
            }
            else if(l.front()->type == ast_type_t::LIST && l.front()->list.size())
            {
                auto f = make_function(l, context, "define");
                context.set(l.front()->list.front()->value, f);
                return f;
            }
            else
                throw eval_error("define: invalid arguments");
        }
        },
        {"lambda", [](const std::list<std::shared_ptr<ASTNode>> &l, Context &context,
                      std::shared_ptr<SchemeFunc>) {
            return make_function(l, context, "lambda");
        }
        },
        {"let",    [](const std::list<std::shared_ptr<ASTNode>> &l, Context &context,
                      std::shared_ptr<SchemeFunc> tail_func) {
            if(l.size() < 2 || l.front()->type != ast_type_t::LIST)
                throw eval_error("let: parameters and code required");
            auto pl = l.front()->list;
            Context local_context = context;
            local_context.newFrame();
            for(auto i : pl)
            {
                if(i->type != ast_type_t::LIST || i->list.size() != 2 || i->list.front()->type != ast_type_t::NAME)
                    throw eval_error("let: list of (name value) required");
                local_context.set(i->list.front()->value, (*next(i->list.begin()))->evaluate(context));
            }
            std::shared_ptr<SchemeObject> res;
            for(auto i = next(l.begin()); i != l.end(); ++i)
            {
                res = (*i)->evaluate(local_context, next(i) == l.end() ? tail_func : nullptr);
            }
            return res;
        }
        },
        {"cond",   [](const std::list<std::shared_ptr<ASTNode>> &l, Context &context,
                      std::shared_ptr<SchemeFunc> tail_func) {
            for(auto branch : l)
            {
                if(branch->type != ast_type_t::LIST || branch->list.size() < 2)
                    throw eval_error("cond: lists with length at least 2 required");
                if((branch->list.front()->type == ast_type_t::NAME && branch->list.front()->value == "else") ||
                   branch->list.front()->evaluate(context)->toBool())
                {
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
        {"if",     [](const std::list<std::shared_ptr<ASTNode>> &l, Context &context,
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
        {"and",    [](const std::list<std::shared_ptr<ASTNode>> &l, Context &context,
                      std::shared_ptr<SchemeFunc> tail_func) {
            if(l.empty())
                throw eval_error("and: non-empty list required");
            std::shared_ptr<SchemeObject> res;
            for(auto i = l.begin(); i != l.end(); ++i)
            {
                if(!(res = (*i)->evaluate(context, next(i) == l.end() ? tail_func : nullptr))->toBool())
                    return scheme_false;
            }
            return res;
        }
        },
        {"or",     [](const std::list<std::shared_ptr<ASTNode>> &l, Context &context,
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
};